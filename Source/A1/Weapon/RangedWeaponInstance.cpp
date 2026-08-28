// Copyright Epic Games, Inc. All Rights Reserved.

#include "Weapon/RangedWeaponInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RangedWeaponInstance)

DEFINE_LOG_CATEGORY(RangedWeaponInstanceLog);

URangedWeaponInstance::URangedWeaponInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void URangedWeaponInstance::OnEquipped()
{
	Super::OnEquipped();
}

void URangedWeaponInstance::OnUnequipped()
{
	Super::OnUnequipped();
}
