#include "A1Ability_MeleeWeaponAttack.h"
#include "Weapon/MeleeWeaponInstance.h"
#include "AbilitySystemComponent.h"
#include "A1GameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/CommonAbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/A1Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_MeleeWeaponAttack)

DEFINE_LOG_CATEGORY(A1Ability_MeleeWeaponAttack);

UA1Ability_MeleeWeaponAttack::UA1Ability_MeleeWeaponAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)

{
	// TODO: Network
}

bool UA1Ability_MeleeWeaponAttack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const UMeleeWeaponInstance* WeaponInstance = GetMeleeWeaponInstance();
	UAnimMontage* AttackMontage = WeaponInstance->GetAttackMontage(ComboIndex);
	if (AttackMontage == nullptr)
	{
		return false;
	}

	return true;
}

void UA1Ability_MeleeWeaponAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ResetHitActors();

	// BP에서 활성화한 경우에만 공격 중 이동 방향으로의 자동 회전을 끈다. (서버·소유 클라 각각에서 로컬 적용)
	if (bDisableOrientRotationDuringAttack)
	{
		// EndAbility에서 원복할 수 있도록 실행되는 쪽의 현재 값을 캐시한다.
		if (const ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
		{
			if (const UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
			{
				bCachedOrientRotationToMovement = MovementComp->bOrientRotationToMovement;
			}
		}
		SetOrientRotationToMovementLocal(false);
	}

	UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
	check(ASC);

	const UMeleeWeaponInstance* WeaponInstance = GetMeleeWeaponInstance();
	UAnimMontage* AttackMontage = WeaponInstance->GetAttackMontage(ComboIndex);

	if (UAbilityTask_WaitGameplayEvent* GameplayEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, A1GameplayTags::GameplayEvent_Trace, nullptr, false, true))
	{
		GameplayEventTask->EventReceived.AddDynamic(this, &ThisClass::OnTargetDataReady);
		GameplayEventTask->ReadyForActivation();
	}

	if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("MeleeAttack"), AttackMontage, 1.0f, NAME_None, false, 1.f, 0.f, false))
	{
		PlayMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
		PlayMontageTask->ReadyForActivation();
	}

	if (UAbilityTask_WaitGameplayEvent* GameplayEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, A1GameplayTags::GameplayEvent_Montage_End, nullptr, true, true))
	{
		GameplayEventTask->EventReceived.AddDynamic(this, &ThisClass::OnMontageEventTriggered);
		GameplayEventTask->ReadyForActivation();
	}
}

void UA1Ability_MeleeWeaponAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 정상 종료·취소 모두 이곳을 거치므로 캐시해 둔 원래 회전 설정으로 되돌린다.
	if (bDisableOrientRotationDuringAttack)
	{
		SetOrientRotationToMovementLocal(bCachedOrientRotationToMovement);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UA1Ability_MeleeWeaponAttack::HandleMontageEvent(FGameplayEventData Payload)
{
	OnMontageFinished();
}

void UA1Ability_MeleeWeaponAttack::OnTargetDataReady(FGameplayEventData Payload)
{
	// AD1EquipmentBase* WeaponActor = const_cast<AD1EquipmentBase*>(Cast<AD1EquipmentBase>(Payload.Instigator));
	// if (WeaponActor == nullptr)
	// 	return;

	UCommonAbilitySystemComponent* SourceASC = GetCommonAbilitySystemComponentFromActorInfo();
	if (SourceASC == nullptr)
		return;

	if (SourceASC->FindAbilitySpecFromHandle(CurrentSpecHandle))
	{
		FGameplayAbilityTargetDataHandle LocalTargetDataHandle(MoveTemp(const_cast<FGameplayAbilityTargetDataHandle&>(Payload.TargetData)));

		TArray<int32> CharacterHitIndexes;
		ParseTargetData(LocalTargetDataHandle, CharacterHitIndexes);

		float Damage = 10.0f; // TODO: Damage Process

		for (int32 CharqacterHitIndex : CharacterHitIndexes)
		{
			FHitResult HitResult = *LocalTargetDataHandle.Data[CharqacterHitIndex]->GetHitResult();
			ProcessHitResult(/*HitResult, Damage, false, nullptr, WeaponActor*/);
			
		}
	}
}

void UA1Ability_MeleeWeaponAttack::OnMontageEventTriggered(FGameplayEventData Payload)
{
	HandleMontageEvent(Payload);
}

void UA1Ability_MeleeWeaponAttack::OnMontageFinished()
{
	if (HasAuthority(&CurrentActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UA1Ability_MeleeWeaponAttack::SetOrientRotationToMovementLocal(bool bNewOrient) const
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

void UA1Ability_MeleeWeaponAttack::ParseTargetData(const FGameplayAbilityTargetDataHandle& InTargetDataHandle, TArray<int32>& OutCharacterHitIndexes)
{
	for (int32 i = 0; i < InTargetDataHandle.Data.Num(); i++)
	{
		const TSharedPtr<FGameplayAbilityTargetData>& TargetData = InTargetDataHandle.Data[i];

		if (FHitResult* HitResult = const_cast<FHitResult*>(TargetData->GetHitResult()))
		{
			if (AActor* HitActor = HitResult->GetActor())
			{
				AA1Character* TargetCharacter = Cast<AA1Character>(HitActor);
				if (TargetCharacter == nullptr)
				{
					TargetCharacter = Cast<AA1Character>(HitActor->GetOwner());
				}

				AActor* SelectedActor = TargetCharacter ? TargetCharacter : HitActor;
				if (CachedHitActors.Contains(SelectedActor))
					continue;

				CachedHitActors.Add(SelectedActor);

				if (TargetCharacter)
				{
					OutCharacterHitIndexes.Add(i);
				}
			}
		}
	}
}

void UA1Ability_MeleeWeaponAttack::ProcessHitResult()
{
	// TODO: Damage 처리
}

void UA1Ability_MeleeWeaponAttack::ResetHitActors()
{
	CachedHitActors.Reset();
}
