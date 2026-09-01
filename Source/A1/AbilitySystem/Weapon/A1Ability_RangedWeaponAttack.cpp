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
#include "AbilitySystem/Tasks/A1AbilityTask_WaitForTick.h"
#include "Actors/A1Projectile.h"
#include "CommonUIExtensionTags.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Game/CommonCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Layout/CommonPrimaryGameLayout.h"
#include "UI/Weapon/A1RangedChargeWidget.h"
#include "Weapon/RangedWeaponInstance.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "Player/A1PlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_RangedWeaponAttack)

DEFINE_LOG_CATEGORY(A1Ability_RangedWeaponAttackLog);

UA1Ability_RangedWeaponAttack::UA1Ability_RangedWeaponAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_StaminaRegen_Blocked);

	// UpdateAimDirectionServer RPC를 쓰려면 어빌리티 인스턴스가 서버·소유 클라 각각 복제되어야 한다.
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
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

	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (ASC == nullptr)
	{
		return false;
	}

	// 발동 즉시 소비되는 스태미나는 없다(0에서 시작해 차징하며 소비). 대신 MinStaminaToStart 이상
	// 보유하고 있어야 홀드를 시작할 수 있다(리젠으로 막 회복된 소량 스태미나로 바로 발동되는 것 방지).
	bool bFoundAttribute = false;
	const float CurrentStamina = UAbilitySystemBlueprintLibrary::GetFloatAttributeFromAbilitySystemComponent(ASC, UA1VitalSet::GetStaminaAttribute(), bFoundAttribute);
	if (bFoundAttribute == false || CurrentStamina < WeaponInstance->GetMinStaminaToStart())
	{
		return false;
	}

	return true;
}

void UA1Ability_RangedWeaponAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	SetOrientRotationToMovement(false);

	if (const ACommonCharacter* Character = GetCommonCharacterFromActorInfo())
	{
		LocalAimDirection = Character->GetActorForwardVector();
		ServerAimDirection = LocalAimDirection;
	}

	// 발동 즉시 소비되는 스태미나는 없다. 스태미나는 차징이 진행되는 동안 TickCharge에서만 소비된다.
	const URangedWeaponInstance* WeaponInstance = GetRangedWeaponInstance();

	StartChargeTimer();

	AimTickTask = UA1AbilityTask_WaitForTick::WaitForTick(this);
	if (AimTickTask != nullptr)
	{
		AimTickTask->OnTick.AddDynamic(this, &ThisClass::OnAimTick);
		AimTickTask->ReadyForActivation();
	}

	const float MaxChargeDuration = WeaponInstance ? WeaponInstance->GetMaxChargeDuration() : 0.f;

	if (IsLocallyControlled())
	{
		ShowChargeWidgetLocal(MaxChargeDuration);
	}

	if (UAbilityTask_WaitInputRelease* ReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true))
	{
		ReleaseTask->OnRelease.AddDynamic(this, &ThisClass::OnInputReleased);
		ReleaseTask->ReadyForActivation();
	}
}

void UA1Ability_RangedWeaponAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	StopChargeTimer();
	SetOrientRotationToMovement(true);

	if (bChargeWidgetVisible)
	{
		HideChargeWidgetLocal();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UA1Ability_RangedWeaponAttack::SetOrientRotationToMovement(bool bRotate)
{
	if (ACommonCharacter* Character = GetCommonCharacterFromActorInfo())
	{
		if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
		{
			MovementComponent->bOrientRotationToMovement = bRotate;
		}
	}
}

void UA1Ability_RangedWeaponAttack::UpdateAimRotationLocal(float DeltaTime)
{
	ACommonCharacter* Character = GetCommonCharacterFromActorInfo();
	if (Character == nullptr)
	{
		return;
	}

	FRotator TargetRotation;

	if (IsLocallyControlled())
	{
		// 마우스 커서 정보는 소유 클라(호스트 포함)에만 있으므로, 소유 클라에서만 실제로 방향을 계산한다.
		AA1PlayerController* PlayerController = Cast<AA1PlayerController>(GetCommonPlayerControllerFromActorInfo());
		if (PlayerController == nullptr)
		{
			return;
		}

		const FVector CharacterLocation = Character->GetActorLocation();
		const FVector CursorLocation = PlayerController->GetCachedCursorLocation();

		TargetRotation = UKismetMathLibrary::FindLookAtRotation(CharacterLocation, CursorLocation);
		TargetRotation.Pitch = 0.f;
		TargetRotation.Roll = 0.f;

		LocalAimDirection = TargetRotation.Vector();

#if ENABLE_DRAW_DEBUG
		
		static constexpr float SpawnHeightOffset = 50.f;
		const FVector SpawnLocation = CharacterLocation + FVector::UpVector * SpawnHeightOffset;
		DrawDebugLine(GetWorld(), SpawnLocation, CursorLocation, FColor::Green, false, -1.f, 0, 2.f);
#endif
	}
	else if (HasAuthority(&CurrentActivationInfo))
	{
		TargetRotation = ServerAimDirection.Rotation();
	}
	else
	{
		return;
	}

	const FRotator NewRotation = FMath::RInterpTo(Character->GetActorRotation(), TargetRotation, DeltaTime, AimRotationInterpSpeed);
	Character->SetActorRotation(NewRotation);
}

void UA1Ability_RangedWeaponAttack::OnAimTick(float DeltaTime)
{
	UpdateAimRotationLocal(DeltaTime);
}

void UA1Ability_RangedWeaponAttack::StopAimTickLocal()
{
	if (AimTickTask != nullptr)
	{
		AimTickTask->EndTask();
		AimTickTask = nullptr;
	}
}

void UA1Ability_RangedWeaponAttack::UpdateAimDirectionServer_Implementation(FVector_NetQuantizeNormal AimDirection)
{
	ServerAimDirection = AimDirection;
}

void UA1Ability_RangedWeaponAttack::SyncAimDirectionToServerLocal()
{
	// Server RPC는 이미 권한이 있는 쪽(호스트/스탠드얼론)에서 호출하면 네트워크를 타지 않고
	// 그 자리에서 곧바로 _Implementation을 실행하므로, HasAuthority 여부와 무관하게 항상 호출해도 된다.
	// (여기서 HasAuthority일 때 호출을 생략하면 호스트 자신의 캐릭터는 ServerAimDirection이
	// ActivateAbility 시점 값에서 갱신되지 않고 굳어버린다)
	if (IsLocallyControlled())
	{
		UpdateAimDirectionServer(LocalAimDirection);
	}
}

void UA1Ability_RangedWeaponAttack::OnInputReleased(float TimeHeld)
{
	StopChargeTimer();

	// 여기서는 조준 회전을 멈추지 않는다. 발사 몽타주가 팔을 들었다 내리는 모션이라, 내리는 순간
	// (Fire 이벤트, OnFireEventReceived)까지는 계속 커서를 따라 돌다가 그 순간 방향이 확정돼야 한다.
	HideChargeWidgetLocal();
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
	// 회전은 여기서 확정해 멈춘다(팔을 내리는 순간의 포즈 고정).
	StopAimTickLocal();

	if (IsLocallyControlled())
	{
		if (AA1PlayerController* PlayerController = Cast<AA1PlayerController>(GetCommonPlayerControllerFromActorInfo()))
		{
			FireProjectileServer(PlayerController->GetCachedCursorLocation());
		}
	}
}

void UA1Ability_RangedWeaponAttack::FireProjectileServer_Implementation(FVector_NetQuantize CursorLocation)
{
	SpawnProjectileAuth(CursorLocation);
}

void UA1Ability_RangedWeaponAttack::OnMontageFinished()
{
	if (HasAuthority(&CurrentActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UA1Ability_RangedWeaponAttack::SpawnProjectileAuth(const FVector& CursorLocation)
{
	if (HasAuthority(&CurrentActivationInfo) == false)
	{
		return;
	}

	const URangedWeaponInstance* WeaponInstance = GetRangedWeaponInstance();
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

	// TODO: 무기 소켓 대신 일단 캐릭터 위치 기준으로 스폰한다. 캐릭터 원점이 바닥(발밑)에 걸려있을
	// 수 있어 살짝 위로 띄운다.
	static constexpr float SpawnHeightOffset = 50.f;
	const FVector SpawnLocation = AvatarPawn->GetActorLocation() + FVector::UpVector * SpawnHeightOffset;

	// 방향 벡터가 아니라 커서의 실제 월드 위치를 받았으므로, 실제 발사 지점(SpawnLocation) 기준으로
	// 직접 방향을 계산한다. 탑다운이라 피치/롤은 0으로 고정(수평 발사).
	FRotator SpawnRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, CursorLocation);
	SpawnRotation.Pitch = 0.f;
	SpawnRotation.Roll = 0.f;

	const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

#if ENABLE_DRAW_DEBUG
	// 디버그: 실제로 받은 커서 위치까지 직접 선을 그어, 발사 방향이 커서 쪽을 향하는지 눈으로 확인한다.
	// (서버 프로세스에서만 렌더링됨: 데디케이티드 서버는 화면이 없어 안 보이므로, PIE에서
	// Listen Server로 띄워야 확인 가능하다)
	DrawDebugLine(GetWorld(), SpawnLocation, CursorLocation, FColor::Red, false, 3.f, 0, 3.f);
#endif

	// 충전 비율(TickCharge가 갱신해온 ChargeAlpha)만큼 0%~100% 충전 값 사이를 보간한다. 데미지는 항상 정수로 반올림.
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
	// 매 활성화마다 차징 상태를 전부 리셋한다. (이전 활성화에서 bIsCharge가 false로 굳은 채 남아있으면
	// 다음 발동에서 TickCharge의 소비 로직 전체가 스킵돼 스태미나가 전혀 안 줄어드는 문제가 있었다)
	ChargeTickElapsed = 0.f;
	PendingStaminaFraction = 0.f;
	ChargeStaminaConsumed = 0.f;
	ChargeAlpha = 0.f;
	bIsCharge = false;

	const URangedWeaponInstance* WeaponInstance = GetRangedWeaponInstance();
	if (WeaponInstance == nullptr || WeaponInstance->GetMaxChargeDuration() <= 0.f)
	{
		// 차징 없는 무기(즉시 발사)는 항상 0% 충전 값으로 나간다.
		return;
	}

	bIsCharge = true;

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

void UA1Ability_RangedWeaponAttack::TickCharge()
{
	// ChargeTickInterval(10Hz) 주기로 서버에 최신 조준 방향을 동기화한다. 실제 화면 회전은
	// OnAimTick(매 프레임)에서 RInterpTo로 부드럽게 처리되므로, 이 주기는 네트워크 전송 빈도만 결정한다.
	SyncAimDirectionToServerLocal();

	if (bIsCharge == false)
	{
		return;
	}

	const URangedWeaponInstance* WeaponInstance = GetRangedWeaponInstance();
	const float MaxChargeDuration = WeaponInstance ? WeaponInstance->GetMaxChargeDuration() : 0.f;
	if (WeaponInstance == nullptr || MaxChargeDuration <= 0.f)
	{
		bIsCharge = false;
		return;
	}

	ChargeTickElapsed += ChargeTickInterval;

	// 0 -> MaxChargeStaminaCost까지 MaxChargeDuration 동안 비율대로(MaxChargeStaminaCost / MaxChargeDuration) 선형 소비한다.
	const float ChargeStaminaBudget = FMath::Max(0.f, WeaponInstance->GetMaxChargeStaminaCost());
	const float DesiredDrain = (ChargeStaminaBudget / MaxChargeDuration) * ChargeTickInterval;
	const float RemainingBudget = FMath::Max(0.f, ChargeStaminaBudget - ChargeStaminaConsumed);

	// 예산은 남아있어도 실제로 보유한 스태미나가 그보다 적을 수 있으므로, 현재 스태미나로도 한 번 더 제한한다.
	// (예: MaxChargeStaminaCost=40, MaxChargeDuration=5초인데 현재 스태미나가 20이면 딱 50%까지만 차징된다)
	UAbilitySystemComponent* ASC = CurrentActorInfo ? CurrentActorInfo->AbilitySystemComponent.Get() : nullptr;
	bool bFoundAttribute = false;
	const float CurrentStamina = ASC ? UAbilitySystemBlueprintLibrary::GetFloatAttributeFromAbilitySystemComponent(ASC, UA1VitalSet::GetStaminaAttribute(), bFoundAttribute) : 0.f;

	float ActualDrain = FMath::Min(DesiredDrain, RemainingBudget);
	if (bFoundAttribute)
	{
		ActualDrain = FMath::Clamp(ActualDrain, 0.f, FMath::Max(0.f, CurrentStamina));
	}

	PendingStaminaFraction += ActualDrain;
	const float WholeAmount = FMath::TruncToFloat(PendingStaminaFraction);
	if (WholeAmount >= 1.f)
	{
		PendingStaminaFraction -= WholeAmount;
		ApplyStaminaCostAmount(WholeAmount);
		ChargeStaminaConsumed += WholeAmount;
	}

	// 충전도는 "시간 진행률"과 "스태미나 예산 소비율" 중 더 작은 쪽을 따른다.
	// 스태미나가 모자라 예산을 다 못 채우면, 시간이 더 지나도 여기서 더 이상 안 올라간다.
	const float TimeRatio = FMath::Clamp(ChargeTickElapsed / MaxChargeDuration, 0.f, 1.f);
	const float StaminaRatio = (ChargeStaminaBudget > 0.f) ? FMath::Clamp(ChargeStaminaConsumed / ChargeStaminaBudget, 0.f, 1.f) : 1.f;
	ChargeAlpha = FMath::Min(TimeRatio, StaminaRatio);

	const bool bTimeMaxed = ChargeTickElapsed >= MaxChargeDuration;
	const bool bBudgetMaxed = ChargeStaminaBudget > 0.f && ChargeStaminaConsumed >= ChargeStaminaBudget;
	const bool bStaminaDepleted = DesiredDrain > 0.f && bFoundAttribute && (CurrentStamina - ActualDrain) <= KINDA_SMALL_NUMBER;

	if (bTimeMaxed || bBudgetMaxed || bStaminaDepleted)
	{
		bIsCharge = false;

		// 시간이 아니라 예산/자원 고갈로 일찍 멈췄다면, 시간 기준으로 계속 재생 중인 위젯 애니메이션을
		// 지금 지점에서 멈춰야 실제 ChargeAlpha(캡핑된 값)와 화면에 보이는 충전 게이지가 일치한다.
		if (!bTimeMaxed)
		{
			PauseChargeWidgetLocal();
		}
	}
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

void UA1Ability_RangedWeaponAttack::PauseChargeWidgetLocal()
{
	if (bChargeWidgetVisible)
	{
		if (UA1RangedChargeWidget* Widget = FindChargeWidgetLocal())
		{
			Widget->PauseCharge();
		}
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

	UCommonActivatableWidgetContainerBase* GameLayer = Layout->GetLayerContainer(CommonUIExtensionTags::UI_Layer_Game);
	UCommonActivatableWidget* ActiveGameWidget = GameLayer ? GameLayer->GetActiveWidget() : nullptr;
	if (ActiveGameWidget == nullptr)
	{
		return nullptr;
	}

	return Layout->FindWidgetOfType<UA1RangedChargeWidget>(ActiveGameWidget);
}
