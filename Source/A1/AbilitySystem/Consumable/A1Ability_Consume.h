// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/CommonGameplayAbility.h"
#include "A1Ability_Consume.generated.h"

class UEquipmentInstance;
class UItemInstance;
class UInventoryComponent;

DECLARE_LOG_CATEGORY_EXTERN(A1Ability_ConsumeLog, Log, All);

/**
 * UA1Ability_Consume
 *
 * 소비형 아이템(포션·음식 등)의 "사용" 어빌리티 베이스.
 * 손에 든(활성) 소비형 장비 인스턴스를 찾고, 사용이 확정되면 원본 아이템을 1개 소비한다.
 * 실제 효과(회복 GE·연출)는 파생 클래스(UA1Ability_DrinkPotion 등)가 담당한다.
 *
 */
UCLASS(Abstract)
class A1_API UA1Ability_Consume : public UCommonGameplayAbility
{
	GENERATED_BODY()

public:
	UA1Ability_Consume(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	/** 현재 사용할 소비형 장비 인스턴스(손에 든/활성 메인 장비)를 반환한다. (없으면 nullptr) */
	UEquipmentInstance* GetConsumableEquipmentInstance() const;

	/** 소비형 장비의 원본 아이템을 1개 소비한다. (스택 감소, 0이 되면 슬롯 해제 후 아이템 제거) */
	void ConsumeSourceItemAuth();

protected:
	/** 이번 활성화에서 아이템이 실제로 사용(소비 확정)되었는지 여부. */
	bool bItemUsed = false;
};
