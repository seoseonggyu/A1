// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Equipment/Fragment/EquipmentFragment.h"
#include "AbilitySystem/Ability/CommonAbilityTypes.h"
#include "GameplayAbilitySpecHandle.h"
#include "EquipmentFragment_Ability.generated.h"

/**
 * 장착 시 GameplayAbility를 부여하는 Fragment
 *
 * 장착 시 지정된 Ability들을 부여하고, 해제 시 제거합니다.
 * InputTag가 지정된 경우 입력 시스템과 연동됩니다.
 * 런타임에 부여된 Ability 핸들을 추적합니다.
 */
USTRUCT(BlueprintType, DisplayName = "Ability")
struct COMMONGAME_API FEquipmentFragment_Ability : public FEquipmentFragment
{
	GENERATED_BODY()

public:
	virtual void OnEquipped(UEquipmentInstance* Instance) override;
	virtual void OnUnequipped(UEquipmentInstance* Instance) override;

public:
	/** 부여할 Ability 목록 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability", meta = (TitleProperty = "InputTag"))
	TArray<FCommonAbilityEntry> Abilities;

private:
	/** 부여된 Ability 핸들 (런타임) */
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> GrantedHandles;
};