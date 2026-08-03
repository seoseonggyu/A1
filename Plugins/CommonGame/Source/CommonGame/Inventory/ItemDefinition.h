// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DataAsset/CommonPrimaryDataAsset.h"
#include "StructUtils/InstancedStruct.h"
#include "Inventory/Fragment/ItemFragment.h"
#include "ItemDefinition.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(ItemDefinitionLog, Log, All);

/**
 * 아이템의 불변 정의를 담는 DataAsset
 *
 * Fragment 배열로 아이템의 특성을 조합하여 정의합니다.
 * CDO로 관리되며, 런타임에 FItemInstance가 이 Definition을 참조합니다.
 */
UCLASS(BlueprintType, Const)
class COMMONGAME_API UItemDefinition : public UCommonPrimaryDataAsset
{
	GENERATED_BODY()

public:
	//-----------------------------------------------------------------------------
	// UPrimaryDataAsset 오버라이드
	//-----------------------------------------------------------------------------

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

public:
	/** 2d 그리드 아이템 Slot 카운트 */
	UPROPERTY(EditDefaultsOnly)
	FIntPoint SlotCount = FIntPoint::ZeroValue;
	
	/** 아이템 표시 이름 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	/** 아이템 설명 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText Description;

	/** Fragment 목록 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fragments")
	TArray<TInstancedStruct<FItemFragment>> Fragments;
};