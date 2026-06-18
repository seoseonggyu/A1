// Copyright Epic Games, Inc. All Rights Reserved.

#include "Equipment/Fragment/EquipmentFragment_AnimationData.h"
#include "Equipment/Fragment/EquipmentFragment.h"
#include "Equipment/EquipmentInstance.h"
#include "Game/CommonCharacter.h"
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
		Character->SetAnimationData(AnimInstanceClass);
	}
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

	Character->ResetAnimationToDefault(); // TODO: Equipment 해제 후 기본 애니메이션 상태로?
}