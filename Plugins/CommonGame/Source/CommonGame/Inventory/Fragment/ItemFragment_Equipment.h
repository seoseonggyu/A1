// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Fragment/ItemFragment.h"
#include "ItemFragment_Equipment.generated.h"

class UEquipmentDefinition;

/**
 * 장비 아이템 Fragment
 *
 * ItemInstance가 장비로 장착될 수 있음을 나타냅니다.
 * EquipmentDefinition에 대한 참조를 보유합니다.
 */
USTRUCT(BlueprintType, DisplayName = "Equipment")
struct COMMONGAME_API FItemFragment_Equipment : public FItemFragment
{
	GENERATED_BODY()

public:
	/** 장비 정의 에셋 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UEquipmentDefinition> EquipmentDefinition;
};