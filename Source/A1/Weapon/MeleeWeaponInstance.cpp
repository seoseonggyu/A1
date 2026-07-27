// Copyright Epic Games, Inc. All Rights Reserved.

#include "Weapon/MeleeWeaponInstance.h"
#include "GameFramework/Pawn.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(MeleeWeaponInstance)

DEFINE_LOG_CATEGORY(MeleeWeaponInstanceLog);

UMeleeWeaponInstance::UMeleeWeaponInstance(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
}

void UMeleeWeaponInstance::OnEquipped()
{
	Super::OnEquipped();

}

void UMeleeWeaponInstance::OnUnequipped()
{

	Super::OnUnequipped();
}
