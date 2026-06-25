// Copyright Epic Games, Inc. All Rights Reserved.

#include "Equipment/Fragment/EquipmentFragment_Ability.h"
#include "Equipment/EquipmentInstance.h"
#include "Game/CommonCharacter.h"
#include "AbilitySystem/CommonAbilitySystemComponent.h"
#include "AbilitySystem/Ability/CommonGameplayAbility.h"
#include "AbilitySystemGlobals.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(EquipmentFragment_Ability)

void FEquipmentFragment_Ability::OnEquipped(UEquipmentInstance* Instance)
{
	if (!Instance->HasAuthority())
	{
		return;
	}

	ACommonCharacter* Character = Instance->GetOwningCharacter();
	if (!Character)
	{
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Character);
	if (!ASC)
	{
		UE_LOG(EquipmentFragmentLog, Warning, TEXT("FEquipmentFragment_Ability: Character에 ASC가 없습니다"));
		return;
	}

	for (const FCommonAbilityEntry& Entry : Abilities)
	{
		if (!Entry.Ability)
		{
			continue;
		}

		FGameplayAbilitySpec Spec(Entry.Ability, 1, INDEX_NONE, Character);

		// InputTag가 있으면 DynamicSpecSourceTags에 추가합니다
		if (Entry.InputTag.IsValid())
		{
			Spec.GetDynamicSpecSourceTags().AddTag(Entry.InputTag);
		}

		FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);

		if (Handle.IsValid())
		{
			GrantedHandles.Add(Handle);
		}
		else
		{
			UE_LOG(EquipmentFragmentLog, Warning, TEXT("FEquipmentFragment_Ability: Ability [%s] 부여 실패"), *GetNameSafe(Entry.Ability));
		}
	}
}

void FEquipmentFragment_Ability::OnUnequipped(UEquipmentInstance* Instance)
{
	if (!Instance->HasAuthority())
	{
		return;
	}

	ACommonCharacter* Character = Instance->GetOwningCharacter();
	if (!Character)
	{
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Character);
	if (!ASC)
	{
		return;
	}

	for (const FGameplayAbilitySpecHandle& Handle : GrantedHandles)
	{
		ASC->ClearAbility(Handle);
	}
	GrantedHandles.Empty();
}