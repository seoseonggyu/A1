// Copyright Epic Games, Inc. All Rights Reserved.
#include "A1Ability_ChangeQuickBarSlot.h"
#include "AbilitySystemComponent.h"
#include "Equipment/EquipmentComponent.h"

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
	
	if (!SlotTag.IsValid())
	{
		return false;
	}

	const UEquipmentComponent* EquipmentComponent = GetEquipmentComponent(ActorInfo);
	if (!EquipmentComponent)
	{
		return false;
	}
	return EquipmentComponent->CanSetActiveSlot(SlotTag);
}

void UA1Ability_ChangeQuickBarSlot::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (UEquipmentComponent* EquipmentComponent = GetEquipmentComponent(ActorInfo))
	{
		EquipmentComponent->SetActiveSlotServer(SlotTag);
	}

	// 슬롯 변경은 즉시 종료
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

UEquipmentComponent* UA1Ability_ChangeQuickBarSlot::GetEquipmentComponent(const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (!ActorInfo)
	{
		return nullptr;
	}
	return UEquipmentComponent::FindEquipmentComponent(Cast<APawn>(GetAvatarActorFromActorInfo()));
}
