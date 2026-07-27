// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Equipment/EquipmentInstance.h"
#include "WeaponInstance.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(WeaponInstanceLog, Log, All);

UCLASS(BlueprintType)
class A1_API UWeaponInstance : public UEquipmentInstance
{
	GENERATED_BODY()

	friend class UEquipmentComponent;

public:
	UWeaponInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};