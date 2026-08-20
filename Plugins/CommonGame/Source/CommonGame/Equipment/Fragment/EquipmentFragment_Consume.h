// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Equipment/Fragment/EquipmentFragment.h"
#include "EquipmentFragment_Consume.generated.h"

class UGameplayEffect;

/**
 * 소비형 아이템(포션 등)이 "사용될 때" 적용할 GameplayEffect를 정의하는 Fragment
 *
 * 장착 즉시 적용되는 FEquipmentFragment_GameplayEffect와 달리, 이 Fragment는 데이터만 보관한다.
 * 소비 어빌리티(UA1Ability_DrinkPotion 등)가 장착된 인스턴스에서 이 Fragment를 찾아
 * ConsumeEffectClass를 소유자에게 적용하고 아이템을 소비한다.
 * 예: HP 포션은 Health를 더하는 GE, MP 포션은 Mana를 더하는 GE를 지정한다.
 */
USTRUCT(BlueprintType, DisplayName = "Consume")
struct COMMONGAME_API FEquipmentFragment_Consume : public FEquipmentFragment
{
	GENERATED_BODY()

public:
	/** 아이템 사용(소비) 시 소유자에게 적용할 GameplayEffect (예: HP/Mana 회복) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consume")
	TSubclassOf<UGameplayEffect> ConsumeEffectClass;
};
