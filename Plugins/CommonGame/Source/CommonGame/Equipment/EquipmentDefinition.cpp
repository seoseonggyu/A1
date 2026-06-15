// Copyright Epic Games, Inc. All Rights Reserved.

#include "Equipment/EquipmentDefinition.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(EquipmentDefinition)

FPrimaryAssetId UEquipmentDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("EquipmentDefinition"), GetFName());
}
