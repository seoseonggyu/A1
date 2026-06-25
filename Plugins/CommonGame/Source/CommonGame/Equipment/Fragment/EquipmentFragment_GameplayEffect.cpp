// Copyright Epic Games, Inc. All Rights Reserved.

#include "Equipment/Fragment/EquipmentFragment_GameplayEffect.h"
#include "Equipment/EquipmentInstance.h"
#include "Game/CommonCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(EquipmentFragment_GameplayEffect)

void FEquipmentFragment_GameplayEffect::OnEquipped(UEquipmentInstance* Instance)
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
		UE_LOG(EquipmentFragmentLog, Warning, TEXT("FEquipmentFragment_GameplayEffect: Character에 ASC가 없습니다"));
		return;
	}

	for (const TSubclassOf<UGameplayEffect>& GEClass : GameplayEffects)
	{
		if (!GEClass)
		{
			continue;
		}

		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		ContextHandle.AddSourceObject(Instance);

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(GEClass, 1.0f, ContextHandle);
		if (SpecHandle.IsValid())
		{
			FActiveGameplayEffectHandle Handle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			if (Handle.IsValid())
			{
				AppliedHandles.Add(Handle);
			}
			else
			{
				UE_LOG(EquipmentFragmentLog, Warning, TEXT("FEquipmentFragment_GameplayEffect: GE [%s] 적용 실패"), *GetNameSafe(GEClass));
			}
		}
	}
}

void FEquipmentFragment_GameplayEffect::OnUnequipped(UEquipmentInstance* Instance)
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

	for (const FActiveGameplayEffectHandle& Handle : AppliedHandles)
	{
		ASC->RemoveActiveGameplayEffect(Handle);
	}
	
	AppliedHandles.Empty();
}