#include "A1Ability_Equipment.h"
#include "Equipment/EquipmentComponent.h"
#include "Weapon/MeleeWeaponInstance.h"
#include "A1GameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_Equipment)

DEFINE_LOG_CATEGORY(A1Ability_Equipment);


UA1Ability_Equipment::UA1Ability_Equipment(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// TODO: 네트워킹 설정
}

bool UA1Ability_Equipment::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	const UMeleeWeaponInstance* WeaponInstance = GetMeleeWeaponInstance();
	if (!WeaponInstance)
	{
		return false;
	}




	return true;
}

void UA1Ability_Equipment::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);


}

void UA1Ability_Equipment::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

UMeleeWeaponInstance* UA1Ability_Equipment::GetMeleeWeaponInstance() const
{
	const APawn* Pawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (!Pawn)
	{
		return nullptr;
	}

	UEquipmentComponent* EquipmentComp = UEquipmentComponent::FindEquipmentComponent(Pawn);
	if (!EquipmentComp)
	{
		return nullptr;
	}


	return Cast<UMeleeWeaponInstance>(EquipmentComp->GetEquipmentInSlot(A1GameplayTags::Equipment_Slot_Weapon));
}
