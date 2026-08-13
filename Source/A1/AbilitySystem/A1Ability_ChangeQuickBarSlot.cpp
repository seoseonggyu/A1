// Copyright Epic Games, Inc. All Rights Reserved.
#include "A1Ability_ChangeQuickBarSlot.h"
#include "AbilitySystemComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_ChangeQuickBarSlot)

UA1Ability_ChangeQuickBarSlot::UA1Ability_ChangeQuickBarSlot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationPolicy = ECommonAbilityActivationPolicy::OnInputTriggered;
	ActivationGroup = ECommonAbilityActivationGroup::Exclusive_Blocking;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
}

bool UA1Ability_ChangeQuickBarSlot::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// SlotTag가 설정되어 있는지 확인
	if (!SlotTag.IsValid())
	{
		return false;
	}

	return true;

}

void UA1Ability_ChangeQuickBarSlot::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{


	// 슬롯 변경은 즉시 종료
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
