// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Skill/A1Ability_Skill_AOE.h"

#include "A1GameplayTags.h"
#include "AbilitySystemComponent.h"
#include "CommonActivatableWidget.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitConfirmCancel.h"
#include "Actors/A1SkillAOEZone.h"
#include "AbilitySystem/Tasks/A1AbilityTask_WaitForTick.h"
#include "CommonUIExtensionTags.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Game/CommonCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Layout/CommonPrimaryGameLayout.h"
#include "Player/A1PlayerController.h"
#include "TimerManager.h"
#include "UI/Weapon/A1SkillTargetingPromptWidget.h"
#include "Weapon/RangedWeaponInstance.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_Skill_AOE)

DEFINE_LOG_CATEGORY(A1Ability_Skill_AOELog);

UA1Ability_Skill_AOE::UA1Ability_Skill_AOE(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetAssetTags(FGameplayTagContainer(A1GameplayTags::Ability_Attack_Skill_1));
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_Skill);

	// 다른 Exclusive 스킬(예: 근접 GroundBreaker)이 조준 중 끼어들지 못하게 막는다.
	// Attack/Interact 같은 Independent 어빌리티는 Status.Skill 태그 관계(다른 스킬들과 동일)로 차단된다.
	ActivationGroup = ECommonAbilityActivationGroup::Exclusive_Blocking;

	// ConfirmAOEServer/UpdateAimDirectionServer RPC를 쓰려면 어빌리티 인스턴스가 서버·소유 클라 각각 복제되어야 한다.
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
}

bool UA1Ability_Skill_AOE::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (GetRangedWeaponInstance() == nullptr)
	{
		return false;
	}

	return true;
}

void UA1Ability_Skill_AOE::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (K2_CheckAbilityCooldown() == false || K2_CheckAbilityCost() == false)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	SetOrientRotationToMovement(false);

	if (const ACommonCharacter* Character = GetCommonCharacterFromActorInfo())
	{
		LocalAimDirection = Character->GetActorForwardVector();
		ServerAimDirection = LocalAimDirection;
		CachedTargetLocation = Character->GetActorLocation();
	}

	bTargetConfirmed = false;

	AimTickTask = UA1AbilityTask_WaitForTick::WaitForTick(this);
	if (AimTickTask != nullptr)
	{
		AimTickTask->OnTick.AddDynamic(this, &ThisClass::OnTargetingTick);
		AimTickTask->ReadyForActivation();
	}

	ConfirmCancelTask = UAbilityTask_WaitConfirmCancel::WaitConfirmCancel(this);
	if (ConfirmCancelTask != nullptr)
	{
		ConfirmCancelTask->OnConfirm.AddDynamic(this, &ThisClass::OnConfirmInput);
		ConfirmCancelTask->OnCancel.AddDynamic(this, &ThisClass::OnCancelInput);
		ConfirmCancelTask->ReadyForActivation();
	}

	if (IsLocallyControlled())
	{
		ShowTargetingPromptLocal();
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(AimSyncTimerHandle, this, &ThisClass::SyncAimDirectionToServerLocal, AimSyncInterval, true);
	}

	// 캐스팅 시작을 몽타주로 알린다. UAbilityTask_PlayMontageAndWait은 ASC를 통해 모든 클라이언트에
	// 복제되므로, 다른 플레이어도 이 캐릭터가 스킬을 준비 중임을 알 수 있다.
	PlayCastMontage();
}

void UA1Ability_Skill_AOE::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	StopAimTickLocal();

	if (ConfirmCancelTask != nullptr)
	{
		ConfirmCancelTask->EndTask();
		ConfirmCancelTask = nullptr;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AimSyncTimerHandle);
	}

	SetOrientRotationToMovement(true);
	HideTargetingPromptLocal();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UA1Ability_Skill_AOE::SetOrientRotationToMovement(bool bRotate)
{
	if (ACommonCharacter* Character = GetCommonCharacterFromActorInfo())
	{
		if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
		{
			MovementComponent->bOrientRotationToMovement = bRotate;
		}
	}
}

void UA1Ability_Skill_AOE::OnTargetingTick(float DeltaTime)
{
	ACommonCharacter* Character = GetCommonCharacterFromActorInfo();
	if (Character == nullptr)
	{
		return;
	}

	FRotator TargetRotation;

	if (IsLocallyControlled())
	{
		AA1PlayerController* PlayerController = Cast<AA1PlayerController>(GetCommonPlayerControllerFromActorInfo());
		if (PlayerController == nullptr)
		{
			return;
		}

		UpdateTargetingLocal(DeltaTime, PlayerController);
		return;
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

void UA1Ability_Skill_AOE::UpdateTargetingLocal(float DeltaTime, AA1PlayerController* PlayerController)
{
	ACommonCharacter* Character = GetCommonCharacterFromActorInfo();
	if (Character == nullptr)
	{
		return;
	}

	const FVector CharacterLocation = Character->GetActorLocation();
	const FVector CursorLocation = PlayerController->GetCachedCursorLocation();

	FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(CharacterLocation, CursorLocation);
	TargetRotation.Pitch = 0.f;
	TargetRotation.Roll = 0.f;

	LocalAimDirection = TargetRotation.Vector();
	CachedTargetLocation = CursorLocation;

	const FRotator NewRotation = FMath::RInterpTo(Character->GetActorRotation(), TargetRotation, DeltaTime, AimRotationInterpSpeed);
	Character->SetActorRotation(NewRotation);

	DrawTargetingIndicatorLocal(CursorLocation);
}

void UA1Ability_Skill_AOE::OnConfirmInput()
{
	ConfirmTargetLocal();
}

void UA1Ability_Skill_AOE::OnCancelInput()
{
	CancelTargetLocal();
}

void UA1Ability_Skill_AOE::StopAimTickLocal()
{
	if (AimTickTask != nullptr)
	{
		AimTickTask->EndTask();
		AimTickTask = nullptr;
	}
}

void UA1Ability_Skill_AOE::UpdateAimDirectionServer_Implementation(FVector_NetQuantizeNormal AimDirection)
{
	ServerAimDirection = AimDirection;
}

void UA1Ability_Skill_AOE::SyncAimDirectionToServerLocal()
{
	// Server RPC는 이미 권한이 있는 쪽(호스트/스탠드얼론)에서 호출하면 그 자리에서 곧바로
	// _Implementation을 실행하므로, HasAuthority 여부와 무관하게 항상 호출해도 된다.
	if (IsLocallyControlled())
	{
		UpdateAimDirectionServer(LocalAimDirection);
	}
}

void UA1Ability_Skill_AOE::ConfirmTargetLocal()
{
	if (bTargetConfirmed)
	{
		return;
	}
	bTargetConfirmed = true;

	// 조준 회전/인디케이터와 확정-취소 태스크를 여기서 바로 정지한다. 비용을 지불하기로 결정한
	// 뒤에는 취소가 불가능해야 하는데, 태스크를 없애두면 이후 취소 입력이 들어와도 반응할 대상이 없다.
	StopAimTickLocal();
	HideTargetingPromptLocal();

	if (ConfirmCancelTask != nullptr)
	{
		ConfirmCancelTask->EndTask();
		ConfirmCancelTask = nullptr;
	}

	// 확정 순간에만 실제로 비용/쿨다운을 커밋한다. 조준만 하다 취소하면 아무 것도 소비되지 않는다.
	if (K2_CommitAbilityCost() == false)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	K2_CommitAbilityCooldown(false, true);
	ConfirmAOEServer(CachedTargetLocation);
	PlaySkillMontage();
}

void UA1Ability_Skill_AOE::CancelTargetLocal()
{
	// EndAbility에서도 정리되지만, 취소 결정과 동시에(스킬 몽타주 대기 없이) 즉시 UI가 사라지도록 여기서도 명시적으로 숨긴다.
	HideTargetingPromptLocal();

	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
}

void UA1Ability_Skill_AOE::PlayCastMontage()
{
	const URangedWeaponInstance* WeaponInstance = GetRangedWeaponInstance();
	UAnimMontage* CastMontage = WeaponInstance ? WeaponInstance->GetAOECastMontage() : nullptr;
	if (CastMontage == nullptr)
	{
		return;
	}

	if (UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("AOECastMontage"), CastMontage, 1.f, NAME_None, true))
	{
		Task->ReadyForActivation();
	}
}

void UA1Ability_Skill_AOE::PlaySkillMontage()
{
	const URangedWeaponInstance* WeaponInstance = GetRangedWeaponInstance();
	UAnimMontage* SkillMontage = WeaponInstance ? WeaponInstance->GetAOESkillMontage() : nullptr;

	UAbilityTask_PlayMontageAndWait* Task = SkillMontage
		? UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("AOESkillMontage"), SkillMontage, 1.f, NAME_None, true)
		: nullptr;

	if (Task == nullptr)
	{
		// 몽타주가 없어도 스킬 자체(장판)는 이미 서버에 요청했으므로, 몽타주 없이 곧바로 어빌리티를 종료한다.
		OnSkillMontageFinished();
		return;
	}

	Task->OnCompleted.AddDynamic(this, &ThisClass::OnSkillMontageFinished);
	Task->OnBlendOut.AddDynamic(this, &ThisClass::OnSkillMontageFinished);
	Task->OnInterrupted.AddDynamic(this, &ThisClass::OnSkillMontageFinished);
	Task->OnCancelled.AddDynamic(this, &ThisClass::OnSkillMontageFinished);
	Task->ReadyForActivation();
}

void UA1Ability_Skill_AOE::OnSkillMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UA1Ability_Skill_AOE::ConfirmAOEServer_Implementation(FVector_NetQuantize TargetLocation)
{
	SpawnAOEZoneAuth(TargetLocation);
}

void UA1Ability_Skill_AOE::SpawnAOEZoneAuth(const FVector& TargetLocation)
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

	TSubclassOf<AA1SkillAOEZone> ZoneClass = WeaponInstance->GetAOEZoneClass();
	if (ZoneClass == nullptr)
	{
		UE_LOG(A1Ability_Skill_AOELog, Warning, TEXT("SpawnAOEZoneAuth: AOEZoneClass가 설정되지 않았습니다. (%s)"), *GetNameSafe(WeaponInstance));
		return;
	}

	APawn* AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (AvatarPawn == nullptr)
	{
		return;
	}

	const FTransform SpawnTransform(FRotator::ZeroRotator, TargetLocation);

	AA1SkillAOEZone* Zone = GetWorld()->SpawnActorDeferred<AA1SkillAOEZone>(ZoneClass, SpawnTransform, AvatarPawn, AvatarPawn, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (Zone != nullptr)
	{
		UAbilitySystemComponent* SourceASC = CurrentActorInfo ? CurrentActorInfo->AbilitySystemComponent.Get() : nullptr;
		Zone->Init(SourceASC, WeaponInstance->GetAOEDamageEffectClass(), WeaponInstance->GetAOESlowEffectClass(), WeaponInstance->GetAOERadius(), WeaponInstance->GetAOETickDamage(),
			WeaponInstance->GetAOETickInterval(), WeaponInstance->GetAOETickCount(), WeaponInstance->GetAOESlowAmount(), WeaponInstance->GetAOESlowDuration());
		Zone->FinishSpawning(SpawnTransform);
	}
}

URangedWeaponInstance* UA1Ability_Skill_AOE::GetRangedWeaponInstance() const
{
	return Cast<URangedWeaponInstance>(GetWeaponInstance());
}

void UA1Ability_Skill_AOE::DrawTargetingIndicatorLocal(const FVector& Center) const
{
#if ENABLE_DRAW_DEBUG
	UWorld* World = GetWorld();
	const URangedWeaponInstance* WeaponInstance = GetRangedWeaponInstance();
	if (World == nullptr || WeaponInstance == nullptr)
	{
		return;
	}

	// 매 프레임 다시 그리므로 한 프레임만 유지한다(LifeTime 0). 커서를 따라다니는 것처럼 보인다.
	DrawDebugCircle(World, Center, WeaponInstance->GetAOERadius(), 32, IndicatorColor, false, 0.f, 0, 2.f,
		FVector(1.f, 0.f, 0.f), FVector(0.f, 1.f, 0.f), false);
#endif
}

void UA1Ability_Skill_AOE::ShowTargetingPromptLocal() const
{
	if (UA1SkillTargetingPromptWidget* Widget = FindTargetingPromptWidgetLocal())
	{
		Widget->ShowPrompt();
	}
	else
	{
		UE_LOG(A1Ability_Skill_AOELog, Warning, TEXT("ShowTargetingPromptLocal: HUD에서 UA1SkillTargetingPromptWidget을 찾을 수 없습니다"));
	}
}

void UA1Ability_Skill_AOE::HideTargetingPromptLocal() const
{
	if (UA1SkillTargetingPromptWidget* Widget = FindTargetingPromptWidgetLocal())
	{
		Widget->HidePrompt();
	}
}

UA1SkillTargetingPromptWidget* UA1Ability_Skill_AOE::FindTargetingPromptWidgetLocal() const
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

	return Layout->FindWidgetOfType<UA1SkillTargetingPromptWidget>(ActiveGameWidget);
}
