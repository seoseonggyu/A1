// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Equipment/EquipmentInstance.h"
#include "WeaponInstance.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(WeaponInstanceLog, Log, All);

/**
 * 무기 인스턴스 베이스 클래스
 *
 * UEquipmentInstance를 상속하여 무기 고유의 기능을 추가합니다.
 */
UCLASS(BlueprintType)
class A1_API UWeaponInstance : public UEquipmentInstance
{
	GENERATED_BODY()

	friend class UEquipmentComponent;

public:
	UWeaponInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};