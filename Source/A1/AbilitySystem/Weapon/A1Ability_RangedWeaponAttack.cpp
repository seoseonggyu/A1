// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Weapon/A1Ability_RangedWeaponAttack.h"

#include "A1GameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CommonActivatableWidget.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "AbilitySystem/A1VitalSet.h"
#include "AbilitySystem/CommonAbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "CommonUIExtensionTags.h"
#include "Equipment/EquipmentComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Layout/CommonPrimaryGameLayout.h"
#include "Actors/A1Projectile.h"
#include "UI/Weapon/A1RangedChargeWidget.h"
#include "Weapon/RangedWeaponInstance.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_RangedWeaponAttack)

DEFINE_LOG_CATEGORY(A1Ability_RangedWeaponAttackLog);

UA1Ability_RangedWeaponAttack::UA1Ability_RangedWeaponAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_StaminaRegen_Blocked);
}

bool UA1Ability_RangedWeaponAttack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const URangedWeaponInstance* WeaponInstance = GetRangedWeaponInstance();
	if (WeaponInstance == nullptr || WeaponInstance->GetAttackMontage() == nullptr)
	{
		return false;
	}

	// 스태미나가 바닥났으면(0 이하) StaminaCost가 0으로 설정돼 있어도 애초에 홀드 자체를 시작할 수 없다.
	// 홀드 시작 후에도 계속 스태미나를 까먹으므로, "지금 0인데 시작은 된다"를 막는 것이 핵심이다.
	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (ASC == nullptr)
	{
		return false;
	}

	bool bFoundAttribute = false;
	const float CurrentStamina = UAbilitySystemBlueprintLibrary::GetFloatAttributeFromAbilitySystemComponent(ASC, UA1VitalSet::GetStaminaAttribute(), bFoundAttribute);
	if (bFoundAttribute == false || CurrentStamina <= 0.f)
	{
		return false;
	}

	// 무기의 스태미나 소비량(기본값)보다 현재 스태미나가 부족하면 발사를 시작할 수 없다.
	const float StaminaCost = WeaponInstance->GetStaminaCost();
	if (StaminaCost > 0.f && CurrentStamina < StaminaCost)
	{
		return false;
	}

	return true;
}

void UA1Ability_RangedWeaponAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const URangedWeaponInstance* WeaponInstance = GetRangedWeaponInstance();

	// 첫 TickCharge 전(예: 즉시 릴리즈)에도 정상적인 방향으로 발사되도록 현재 정면으로 초기화한다.
	if (const APawn* AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo()))
	{
		ServerAimDirection = AvatarPawn->GetActorForwardVector();
	}

	// 홀드 시작 즉시 기본(0% 충전) 스태미나를 소비한다. 나머지는 StartChargeTimer가 홀드 중 나눠서 소비한다.
	if (WeaponInstance != nullptr)
	{
		ApplyStaminaCostAmount(WeaponInstance->GetStaminaCost());
	}
	StartChargeTimer();

	// 차징 중(홀드~발사 몽타주 재생까지)에는 이동 방향으로의 자동 회전만 막는다. 서버·소유 클라 각각 로컬로 적용.
	if (bDisableOrientRotationDuringCharge)
	{
		if (const ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
		{
			if (const UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
			{
				bCachedOrientRotationToMovement = MovementComp->bOrientRotationToMovement;
			}
		}
		SetOrientRotationToMovementLocal(false);
	}

	const float MaxChargeDuration = WeaponInstance ? WeaponInstance->GetMaxChargeDuration() : 0.f;

	// 차징 UI는 순수 로컬 연출이므로 로컬로 조작되는 쪽에서만 띄운다.
	// (리슨 서버 호스트는 HasAuthority()도 true이므로 권한 여부가 아니라 IsLocallyControlled()로 판단해야 한다)
	if (IsLocallyControlled())
	{
		ShowChargeWidgetLocal(MaxChargeDuration);
	}

	// 즉시 발사하지 않고 입력을 뗄 때까지 기다린다. 홀드 시간(TimeHeld)이 곧 충전 시간이다.
	if (UAbilityTask_WaitInputRelease* ReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true))
	{
		ReleaseTask->OnRelease.AddDynamic(this, &ThisClass::OnInputReleased);
		ReleaseTask->ReadyForActivation();
	}
}

void UA1Ability_RangedWeaponAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	StopChargeTimer();

	// 정상 종료·취소 모두 이곳을 거치므로 캐시해 둔 원래 회전 설정으로 되돌린다.
	if (bDisableOrientRotationDuringCharge)
	{
		SetOrientRotationToMovementLocal(bCachedOrientRotationToMovement);
	}

	if (bChargeWidgetVisible)
	{
		HideChargeWidgetLocal();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UA1Ability_RangedWeaponAttack::OnInputReleased(float TimeHeld)
{
	StopChargeTimer();
	HideChargeWidgetLocal();

	const URangedWeaponInstance* WeaponInstance = GetRangedWeaponInstance();
	const float MaxChargeDuration = WeaponInstance ? WeaponInstance->GetMaxChargeDuration() : 0.f;

	ChargeAlpha = (MaxChargeDuration > 0.f) ? FMath::Clamp(TimeHeld / MaxChargeDuration, 0.f, 1.f) : 0.f;

	PlayFireMontage();
}

void UA1Ability_RangedWeaponAttack::PlayFireMontage()
{
	const URangedWeaponInstance* WeaponInstance = GetRangedWeaponInstance();
	UAnimMontage* AttackMontage = WeaponInstance ? WeaponInstance->GetAttackMontage() : nullptr;

	if (UAbilityTask_WaitGameplayEvent* FireEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, A1GameplayTags::GameplayEvent_Weapon_Fire, nullptr, false, true))
	{
		FireEventTask->EventReceived.AddDynamic(this, &ThisClass::OnFireEventReceived);
		FireEventTask->ReadyForActivation();
	}

	if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("RangedAttack"), AttackMontage, 1.0f, NAME_None, false, 1.f, 0.f, false))
	{
		PlayMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
		PlayMontageTask->ReadyForActivation();
	}
}

void UA1Ability_RangedWeaponAttack::OnFireEventReceived(FGameplayEventData Payload)
{
	SpawnProjectileAuth();
}

void UA1Ability_RangedWeaponAttack::OnMontageFinished()
{
	if (HasAuthority(&CurrentActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

// GameplayEvent.Weapon.Fire는 몽타주를 재생 중인 쪽(서버·소유 클라 각각)에서 로컬로 발생하므로
// 이 콜백 자체는 양쪽에서 다 불린다. 실제 스폰은 서버 권위에서만 수행한다.
void UA1Ability_RangedWeaponAttack::SpawnProjectileAuth()
{
	if (HasAuthority(&CurrentActivationInfo) == false)
	{
		return;
	}

	URangedWeaponInstance* WeaponInstance = GetRangedWeaponInstance();
	if (WeaponInstance == nullptr)
	{
		return;
	}

	TSubclassOf<AA1Projectile> ProjectileClass = WeaponInstance->GetProjectileClass();
	if (ProjectileClass == nullptr)
	{
		UE_LOG(A1Ability_RangedWeaponAttackLog, Warning, TEXT("SpawnProjectileAuth: ProjectileClass가 설정되지 않았습니다. (%s)"), *GetNameSafe(WeaponInstance));
		return;
	}

	APawn* AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (AvatarPawn == nullptr)
	{
		return;
	}

	// 무기 액터(스태프 메시)는 서버·클라 각각 로컬로 스폰돼 있으므로(EquipmentInstance::SpawnEquipmentActors),
	// 서버에서도 그대로 조회해 발사 소켓 위치를 구할 수 있다.
	FVector SpawnLocation = AvatarPawn->GetActorLocation();

	// 캐릭터/컨트롤러의 실제 Rotation이 아니라 UpdateAimDirectionServer로 전달받아 서버가 들고 있는
	// ServerAimDirection을 쓴다(마우스 조준 정보는 소유 클라에만 있어 순수 데이터로만 전달받음).
	const FRotator SpawnRotation = ServerAimDirection.Rotation();

	UEquipmentComponent* EquipmentComp = UEquipmentComponent::FindEquipmentComponent(AvatarPawn);
	AActor* WeaponActor = EquipmentComp ? EquipmentComp->GetEquipmentInstance(A1GameplayTags::Equipment_Slot_Weapon) : nullptr;
	if (WeaponActor != nullptr)
	{
		const FName SocketName = WeaponInstance->GetProjectileSocketName();
		if (USkeletalMeshComponent* WeaponMesh = WeaponActor->FindComponentByClass<USkeletalMeshComponent>())
		{
			if (WeaponMesh->DoesSocketExist(SocketName))
			{
				SpawnLocation = WeaponMesh->GetSocketLocation(SocketName);
			}
		}
	}

	const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

	// 충전 비율(OnInputReleased에서 확정)만큼 0%~100% 충전 값 사이를 보간한다. 데미지는 항상 정수로 반올림.
	const float Damage = FMath::RoundToFloat(FMath::Lerp(WeaponInstance->GetBaseDamage(), WeaponInstance->GetMaxChargeDamage(), ChargeAlpha));
	const float Speed = FMath::Lerp(WeaponInstance->GetProjectileSpeed(), WeaponInstance->GetMaxChargeSpeed(), ChargeAlpha);
	const float Scale = FMath::Lerp(1.f, WeaponInstance->GetMaxChargeScale(), ChargeAlpha);

	AA1Projectile* Projectile = GetWorld()->SpawnActorDeferred<AA1Projectile>(ProjectileClass, SpawnTransform, AvatarPawn, AvatarPawn, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (Projectile != nullptr)
	{
		UAbilitySystemComponent* SourceASC = CurrentActorInfo ? CurrentActorInfo->AbilitySystemComponent.Get() : nullptr;
		Projectile->Init(SourceASC, DamageEffectClass, Damage, Speed, Scale);
		Projectile->FinishSpawning(SpawnTransform);
	}
}

URangedWeaponInstance* UA1Ability_RangedWeaponAttack::GetRangedWeaponInstance() const
{
	return Cast<URangedWeaponInstance>(GetWeaponInstance());
}

// Amount만큼 SetByCaller로 실어 스태미나 소비 GE를 자신에게 적용한다. 소비량은 항상 정수로 반올림한다
// (틱마다 나눠 소비하다 보면 소수점 값이 나오는데, 스태미나 표시/계산을 정수로만 다루기 위함).
// (UA1Ability_MeleeWeaponAttack::ApplyStaminaCost와 동일한 방식 - 홀드 시작 즉시 1회, 이후 TickCharge에서 조금씩 반복 호출된다)
void UA1Ability_RangedWeaponAttack::ApplyStaminaCostAmount(float Amount) const
{
	const float RoundedAmount = FMath::RoundToFloat(Amount);
	if (StaminaCostEffectClass == nullptr || RoundedAmount <= 0.f)
	{
		return;
	}

	UCommonAbilitySystemComponent* SourceASC = GetCommonAbilitySystemComponentFromActorInfo();
	if (SourceASC == nullptr)
	{
		return;
	}

	FScopedPredictionWindow ScopedPrediction(SourceASC, GetCurrentActivationInfo().GetActivationPredictionKey());

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(StaminaCostEffectClass);
	if (SpecHandle.IsValid() == false)
	{
		return;
	}

	SpecHandle.Data->SetSetByCallerMagnitude(A1GameplayTags::SetByCaller_StaminaCost, -RoundedAmount);
	(void)ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle);
}

void UA1Ability_RangedWeaponAttack::StartChargeTimer()
{
	const URangedWeaponInstance* WeaponInstance = GetRangedWeaponInstance();
	if (WeaponInstance == nullptr || WeaponInstance->GetMaxChargeDuration() <= 0.f)
	{
		return;
	}

	ChargeTickElapsed = 0.f;
	PendingStaminaFraction = 0.f;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ChargeTickTimerHandle, this, &ThisClass::TickCharge, ChargeTickInterval, true);
	}
}

void UA1Ability_RangedWeaponAttack::StopChargeTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ChargeTickTimerHandle);
	}
}

// ChargeTickInterval마다 호출된다. 조준 방향은 소유 클라에서만(마우스 정보가 거기에만 있으므로) 갱신하고,
// 스태미나는 (MaxChargeStaminaCost - StaminaCost)를 MaxChargeDuration에 걸쳐 나눈 만큼을 계속 소비한다.
// 최대 충전에 도달하면(홀드를 그 이상 유지해도 더 세지지 않으므로) 스스로 타이머를 멈춘다.
void UA1Ability_RangedWeaponAttack::TickCharge()
{
	if (IsLocallyControlled())
	{
		UpdateAimDirectionLocal();
	}

	const URangedWeaponInstance* WeaponInstance = GetRangedWeaponInstance();
	const float MaxChargeDuration = WeaponInstance ? WeaponInstance->GetMaxChargeDuration() : 0.f;
	if (WeaponInstance == nullptr || MaxChargeDuration <= 0.f)
	{
		StopChargeTimer();
		return;
	}

	ChargeTickElapsed += ChargeTickInterval;

	// 틱당 소비량이 1보다 작으면(예: 0.2) 매번 반올림해서 적용하면 계속 0으로 버려질 수 있으므로,
	// 소수점을 이월(PendingStaminaFraction)했다가 1 이상 모였을 때만 정수로 소비한다.
	const float DrainPerSecond = (WeaponInstance->GetMaxChargeStaminaCost() - WeaponInstance->GetStaminaCost()) / MaxChargeDuration;
	PendingStaminaFraction += DrainPerSecond * ChargeTickInterval;

	const float WholeAmount = FMath::TruncToFloat(PendingStaminaFraction);
	if (WholeAmount >= 1.f)
	{
		PendingStaminaFraction -= WholeAmount;
		ApplyStaminaCostAmount(WholeAmount);
	}

	if (ChargeTickElapsed >= MaxChargeDuration)
	{
		StopChargeTimer();
	}
}

// 소유 클라 전용. 탑다운 카메라 레이(마우스 커서 방향)와 캐릭터 높이의 수평면이 만나는 지점을
// 조준 지점으로 삼는다. 캐릭터/컨트롤러 회전은 절대 건드리지 않고(이 프로젝트의 탑다운 카메라가
// 캐릭터 회전에 종속돼 있어, 예전에 SetControlRotation으로 실제 회전시켰더니 차징 중 화면이
// 캐릭터를 따라 돌면서 마우스 커서 투영이 깨지는 문제가 있었다), 방향 값만 서버로 전달한다.
void UA1Ability_RangedWeaponAttack::UpdateAimDirectionLocal()
{
	APawn* AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	APlayerController* PC = Cast<APlayerController>(GetControllerFromActorInfo());
	if (AvatarPawn == nullptr || PC == nullptr)
	{
		return;
	}

	FVector CursorWorldLocation;
	FVector CursorWorldDirection;
	if (PC->DeprojectMousePositionToWorld(CursorWorldLocation, CursorWorldDirection) == false)
	{
		return;
	}

	if (FMath::IsNearlyZero(CursorWorldDirection.Z))
	{
		return;
	}

	const float PlaneZ = AvatarPawn->GetActorLocation().Z;
	const float T = (PlaneZ - CursorWorldLocation.Z) / CursorWorldDirection.Z;
	if (T <= 0.f)
	{
		return;
	}

	const FVector AimPoint = CursorWorldLocation + CursorWorldDirection * T;
	FVector AimDirection = AimPoint - AvatarPawn->GetActorLocation();
	AimDirection.Z = 0.f;

	if (AimDirection.IsNearlyZero())
	{
		return;
	}

	UpdateAimDirectionServer(AimDirection.GetSafeNormal());
}

void UA1Ability_RangedWeaponAttack::UpdateAimDirectionServer_Implementation(FVector_NetQuantizeNormal InAimDirection)
{
	ServerAimDirection = InAimDirection;
}

void UA1Ability_RangedWeaponAttack::SetOrientRotationToMovementLocal(bool bNewOrient) const
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		return;
	}

	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (!MovementComp)
	{
		return;
	}

	MovementComp->bOrientRotationToMovement = bNewOrient;
}

void UA1Ability_RangedWeaponAttack::ShowChargeWidgetLocal(float MaxChargeDuration)
{
	if (UA1RangedChargeWidget* Widget = FindChargeWidgetLocal())
	{
		Widget->StartCharge(MaxChargeDuration);
		bChargeWidgetVisible = true;
	}
	else
	{
		UE_LOG(A1Ability_RangedWeaponAttackLog, Warning, TEXT("ShowChargeWidgetLocal: HUD에서 UA1RangedChargeWidget을 찾을 수 없습니다"));
	}
}

void UA1Ability_RangedWeaponAttack::HideChargeWidgetLocal()
{
	bChargeWidgetVisible = false;

	if (UA1RangedChargeWidget* Widget = FindChargeWidgetLocal())
	{
		Widget->StopCharge();
	}
}

UA1RangedChargeWidget* UA1Ability_RangedWeaponAttack::FindChargeWidgetLocal() const
{
	APlayerController* PC = Cast<APlayerController>(GetControllerFromActorInfo());
	UCommonPrimaryGameLayout* Layout = PC ? UCommonPrimaryGameLayout::GetPrimaryGameLayout(PC->GetLocalPlayer()) : nullptr;
	if (Layout == nullptr)
	{
		return nullptr;
	}

	// UA1Ability_Interact_Hold::FindHoldWidgetLocal과 동일한 이유로, Layout 자신의 WidgetTree가 아니라
	// UI.Layer.Game에 Push된 위젯(W_GameLayout)의 "현재 활성 위젯" 트리 안에서 찾아야 한다.
	UCommonActivatableWidgetContainerBase* GameLayer = Layout->GetLayerContainer(CommonUIExtensionTags::UI_Layer_Game);
	UCommonActivatableWidget* ActiveGameWidget = GameLayer ? GameLayer->GetActiveWidget() : nullptr;
	if (ActiveGameWidget == nullptr)
	{
		return nullptr;
	}

	return Layout->FindWidgetOfType<UA1RangedChargeWidget>(ActiveGameWidget);
}
