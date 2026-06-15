// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EquipmentFragment.generated.h"

class UEquipmentInstance;

DECLARE_LOG_CATEGORY_EXTERN(EquipmentFragmentLog, Log, All);

/**
 * 장비의 개별 속성을 정의하는 Fragment 베이스 구조체
 *
 * EquipmentDefinition에 템플릿으로 정의되고, 장착 시 EquipmentInstance에 복사됩니다.
 * 런타임 상태(부여된 어빌리티 핸들 등)를 추적할 수 있습니다.
 */
USTRUCT(BlueprintType, meta = (Hidden))
struct COMMONGAME_API FEquipmentFragment
{
	GENERATED_BODY()

public:
	virtual ~FEquipmentFragment() = default;

	/** 장비가 장착될 때 호출됩니다 */
	virtual void OnEquipped(UEquipmentInstance* Instance) {}

	/** 장비가 해제될 때 호출됩니다 */
	virtual void OnUnequipped(UEquipmentInstance* Instance) {}
};
