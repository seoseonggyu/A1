#include "A1Ability_MeleeWeaponAttack.h"
#include "Weapon/MeleeWeaponInstance.h"
#include "AbilitySystemComponent.h"
#include "A1GameplayTags.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_MeleeWeaponAttack)

DEFINE_LOG_CATEGORY(A1Ability_MeleeWeaponAttack);

UA1Ability_MeleeWeaponAttack::UA1Ability_MeleeWeaponAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)

{
	// TODO: Network
}

bool UA1Ability_MeleeWeaponAttack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);

	return true;
}

void UA1Ability_MeleeWeaponAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const UMeleeWeaponInstance* WeaponInstance = GetMeleeWeaponInstance();
	UAnimMontage* AttackMontage = WeaponInstance->GetAttackMontage(ComboIndex);
	if (AttackMontage == nullptr)
	{
		K2_EndAbility();
	}

	UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
	check(ASC);

	ASC->PlayMontage(this, ActivationInfo, AttackMontage, 1.0f);

	// TODO: Not This End
	// K2_EndAbility();
}

void UA1Ability_MeleeWeaponAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
