#include "A1Ability_Skill_WhirlwindSlash.h"

#include "A1GameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/CommonAbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_Skill_WhirlwindSlash)

// TODO: 몽타주 Notify

UA1Ability_Skill_WhirlwindSlash::UA1Ability_Skill_WhirlwindSlash(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetAssetTags(FGameplayTagContainer(A1GameplayTags::Ability_Attack_Skill_1));
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_RejectHitReact);
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_Skill);
	
}

void UA1Ability_Skill_WhirlwindSlash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (K2_CheckAbilityCooldown() == false || K2_CheckAbilityCost() == false)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo) == false)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	// 시전 시작(선딜) 구간에서는 이동 입력을 막는다.
	// 이동 모드는 유지되므로 몽타주 루트 모션은 그대로 캐릭터를 움직인다.
	SetMoveInputBlockedLocal(true);
	
	if (UAbilityTask_PlayMontageAndWait* WhirlwindSlashMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("WhirlwindSlashMontage"), WhirlwindSlashMontage, 1.f, NAME_None, true))
	{
		WhirlwindSlashMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
		WhirlwindSlashMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageFinished);
		WhirlwindSlashMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageFinished);
		WhirlwindSlashMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageFinished);
		WhirlwindSlashMontageTask->ReadyForActivation();
	}

	if (UAbilityTask_WaitGameplayEvent* TraceEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, A1GameplayTags::GameplayEvent_Trace, nullptr, false, true))
	{
		TraceEventTask->EventReceived.AddDynamic(this, &ThisClass::OnTrace);
		TraceEventTask->ReadyForActivation();
	}

	if (UAbilityTask_WaitGameplayEvent* ResetEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, A1GameplayTags::GameplayEvent_Reset, nullptr, false, true))
	{
		ResetEventTask->EventReceived.AddDynamic(this, &ThisClass::OnReset);
		ResetEventTask->ReadyForActivation();
	}

	if (UAbilityTask_WaitGameplayEvent* WhirlwindSlashBeginEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, A1GameplayTags::GameplayEvent_Montage_Begin, nullptr, true, true))
	{
		WhirlwindSlashBeginEventTask->EventReceived.AddDynamic(this, &ThisClass::OnWhirlwindSlashBegin);
		WhirlwindSlashBeginEventTask->ReadyForActivation();
	}
	
	if (UAbilityTask_WaitGameplayEvent* WhirlwindSlashBeginEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, A1GameplayTags::GameplayEvent_Montage_Move, nullptr, true, true))
	{
		WhirlwindSlashBeginEventTask->EventReceived.AddDynamic(this, &ThisClass::OnMove);
		WhirlwindSlashBeginEventTask->ReadyForActivation();
	}

	if (UAbilityTask_WaitGameplayEvent* WhirlwindSlashEndEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, A1GameplayTags::GameplayEvent_Montage_End, nullptr, true, true))
	{
		WhirlwindSlashEndEventTask->EventReceived.AddDynamic(this, &ThisClass::OnWhirlwindSlashEnd);
		WhirlwindSlashEndEventTask->ReadyForActivation();
	}
}

void UA1Ability_Skill_WhirlwindSlash::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 정상 종료·취소·중단 모두 이곳을 거치므로 막아두었던 이동 입력을 반드시 원복한다.
	SetMoveInputBlockedLocal(false);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UA1Ability_Skill_WhirlwindSlash::OnMove(FGameplayEventData Payload)
{
	SetMoveInputBlockedLocal(false);
}

void UA1Ability_Skill_WhirlwindSlash::OnTrace(FGameplayEventData Payload)
{
	UCommonAbilitySystemComponent* SourceASC = GetCommonAbilitySystemComponentFromActorInfo();
	if (SourceASC == nullptr)
		return;

	if (SourceASC->FindAbilitySpecFromHandle(CurrentSpecHandle))
	{
		FGameplayAbilityTargetDataHandle LocalTargetDataHandle(MoveTemp(const_cast<FGameplayAbilityTargetDataHandle&>(Payload.TargetData)));

		TArray<int32> CharacterHitIndexes;
		ParseTargetData(LocalTargetDataHandle, CharacterHitIndexes);

		for (int32 CharacterHitIndex : CharacterHitIndexes)
		{
			FHitResult HitResult = *LocalTargetDataHandle.Data[CharacterHitIndex]->GetHitResult();
			ProcessHitResult(HitResult, Damage);
		}
	}
}

void UA1Ability_Skill_WhirlwindSlash::OnReset(FGameplayEventData Payload)
{
	ResetHitActors();
}

void UA1Ability_Skill_WhirlwindSlash::OnWhirlwindSlashBegin(FGameplayEventData Payload)
{

}

void UA1Ability_Skill_WhirlwindSlash::OnWhirlwindSlashEnd(FGameplayEventData Payload)
{
	SetMoveInputBlockedLocal(true);
}

void UA1Ability_Skill_WhirlwindSlash::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
