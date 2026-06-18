// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Equipment/Fragment/EquipmentFragment.h"
#include "EquipmentFragment_AnimationData.generated.h"

class UCharacterAnimationData;

/**
 * 장착 시 애니메이션 데이터를 설정하는 Fragment
 *
 * 장착 시 AnimationData를 적용하고, 해제 시 기본값으로 복구합니다.
 */
USTRUCT(BlueprintType, DisplayName = "Animation Data")
struct COMMONGAME_API FEquipmentFragment_AnimationData : public FEquipmentFragment
{
	GENERATED_BODY()

public:
	virtual void OnEquipped(UEquipmentInstance* Instance) override;
	virtual void OnUnequipped(UEquipmentInstance* Instance) override;

public:
	/** 적용할 애니메이션 데이터 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (AssetBundles = "Client"))
	TSubclassOf<UAnimInstance> AnimInstanceClass;
};