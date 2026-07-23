// Copyright Epic Games, Inc. All Rights Reserved.

#include "A1Ability_MeleeWeaponComboAttack.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitConfirmCancel.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Tasks/A1AbilityTask_WaitInputStart.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_MeleeWeaponComboAttack)

DEFINE_LOG_CATEGORY(A1Ability_MeleeWeaponComboAttack);

UA1Ability_MeleeWeaponComboAttack::UA1Ability_MeleeWeaponComboAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationGroup = ECommonAbilityActivationGroup::Exclusive_Replaceable;
}

bool UA1Ability_MeleeWeaponComboAttack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);

}

void UA1Ability_MeleeWeaponComboAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	bInputPressed = false;
	bInputReleased = false;
	
	if (auto* InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true))
	{
		InputReleaseTask->OnRelease.AddDynamic(this, &ThisClass::OnInputRelease);
		InputReleaseTask->ReadyForActivation();
	}
	
	WaitInputContinue();
	WaitInputStop();
}

void UA1Ability_MeleeWeaponComboAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UA1Ability_MeleeWeaponComboAttack::HandleMontageEvent(FGameplayEventData Payload)
{
	if (HasAuthority(&CurrentActivationInfo) == false)
		return;
	
	bool bCanContinue = NextAbilityClass && (bInputPressed || bInputReleased == false);
	if (bCanContinue)
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
		{
			AbilitySystemComponent->TryActivateAbilityByClass(NextAbilityClass);
		}
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UA1Ability_MeleeWeaponComboAttack::WaitInputContinue()
{
	if (UA1AbilityTask_WaitInputStart* InputStartTask = UA1AbilityTask_WaitInputStart::WaitInputStart(this))
	{
		InputStartTask->OnStart.AddDynamic(this, &ThisClass::OnInputStart);
		InputStartTask->ReadyForActivation();
	}
}

void UA1Ability_MeleeWeaponComboAttack::WaitInputStop()
{
	if (UAbilityTask_WaitConfirmCancel* InputConfirmCancelTask = UAbilityTask_WaitConfirmCancel::WaitConfirmCancel(this))
	{
		InputConfirmCancelTask->OnCancel.AddDynamic(this, &ThisClass::OnInputCancel);
		InputConfirmCancelTask->ReadyForActivation();
	}
}

void UA1Ability_MeleeWeaponComboAttack::OnInputRelease(float TimeHeld)
{
	bInputReleased = true;
}


void UA1Ability_MeleeWeaponComboAttack::OnInputStart()
{
	bInputPressed = true;
	WaitInputContinue();
}

void UA1Ability_MeleeWeaponComboAttack::OnInputCancel()
{
	bInputPressed = false;
	WaitInputStop();
}
