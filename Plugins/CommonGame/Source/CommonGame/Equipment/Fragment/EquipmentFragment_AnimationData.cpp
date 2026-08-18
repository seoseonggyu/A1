// Copyright Epic Games, Inc. All Rights Reserved.

#include "Equipment/Fragment/EquipmentFragment_AnimationData.h"
#include "Equipment/EquipmentInstance.h"
#include "Game/CommonCharacter.h"

#include "Awaiters/Asset.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(EquipmentFragment_AnimationData)

void FEquipmentFragment_AnimationData::OnEquipped(UEquipmentInstance* Instance)
{
	// 서버에서는 애니메이션이 불필요합니다
	if (Instance->HasAuthority())
	{
		return;
	}

	ACommonCharacter* Character = Instance->GetOwningCharacter();
	if (!Character)
	{
		return;
	}

	if (AnimInstanceClass) {
		Character->SetAnimationData(AnimInstanceClass, Instance);
	}

	LoadAnimMontageCoroutine(Character, EquipMontage);

}

void FEquipmentFragment_AnimationData::OnUnequipped(UEquipmentInstance* Instance)
{
	// 서버에서는 애니메이션이 불필요합니다
	if (Instance->HasAuthority())
	{
		return;
	}

	ACommonCharacter* Character = Instance->GetOwningCharacter();
	if (!Character)
	{
		return;
	}

	Character->ResetAnimationToDefault(Instance);
}

TCoroTask<void> FEquipmentFragment_AnimationData::LoadAnimMontageCoroutine(ACommonCharacter* Character, TSoftObjectPtr<UAnimMontage> Montage)
{
	if (Character == nullptr)
	{
		co_return;
	}

	UAnimMontage* LoadedMontage = co_await Coro::Async::LoadObject(Character, Montage);
	if (LoadedMontage)
	{
		Character->PlayAnimMontage(LoadedMontage);
	}
}
