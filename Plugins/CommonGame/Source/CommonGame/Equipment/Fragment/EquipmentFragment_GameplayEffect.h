// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Equipment/Fragment/EquipmentFragment.h"
#include "ActiveGameplayEffectHandle.h"
#include "EquipmentFragment_GameplayEffect.generated.h"

class UGameplayEffect;

/**
 * 장착 시 GameplayEffect를 적용하는 Fragment
 *
 * 장착 시 지정된 GE들을 적용하고, 해제 시 제거합니다.
 * 런타임에 적용된 GE 핸들을 추적합니다.
 */
USTRUCT(BlueprintType, DisplayName = "Gameplay Effect")
struct COMMONGAME_API FEquipmentFragment_GameplayEffect : public FEquipmentFragment
{
	GENERATED_BODY()

public:
	virtual void OnEquipped(UEquipmentInstance* Instance) override;
	virtual void OnUnequipped(UEquipmentInstance* Instance) override;

public:
	/** 적용할 GameplayEffect 목록 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Effect")
	TArray<TSubclassOf<UGameplayEffect>> GameplayEffects;

private:
	/** 적용된 GE 핸들 (런타임) */
	UPROPERTY(NotReplicated)
	TArray<FActiveGameplayEffectHandle> AppliedHandles;
};