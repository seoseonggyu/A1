// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DataAsset/CommonPrimaryDataAsset.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "Equipment/Fragment/EquipmentFragment.h"
#include "EquipmentDefinition.generated.h"

class UEquipmentInstance;

//-----------------------------------------------------------------------------
// FEquipmentActorToSpawn
//-----------------------------------------------------------------------------

/**
 * 장착 시 스폰할 액터 정보
 */
USTRUCT(BlueprintType)
struct COMMONGAME_API FEquipmentActorToSpawn
{
	GENERATED_BODY()

public:
	/** 스폰할 Actor 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AActor> ActorClass;

	/** 장착 위치 소켓 이름 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName AttachSocket = NAME_None;

	/** 소켓 기준 Transform 오프셋 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FTransform AttachTransform;
};

//-----------------------------------------------------------------------------
// UEquipmentDefinition
//-----------------------------------------------------------------------------

/**
 * 장비의 불변 정의를 담는 DataAsset
 *
 * 장착 시 스폰할 Actor 클래스와 Fragment를 정의합니다.
 * ItemDefinition의 FFragment_Equippable에서 참조됩니다.
 */
UCLASS(BlueprintType, Const)
class COMMONGAME_API UEquipmentDefinition : public UCommonPrimaryDataAsset
{
	GENERATED_BODY()

public:
	//-----------------------------------------------------------------------------
	// UPrimaryDataAsset 오버라이드
	//-----------------------------------------------------------------------------

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	//-----------------------------------------------------------------------------
	// Fragment API
	//-----------------------------------------------------------------------------

	/** Fragment를 검색합니다 */
	template<typename T>
	const T* FindFragment() const
	{
		for (const TInstancedStruct<FEquipmentFragment>& Entry : Fragments)
		{
			if (const T* Found = Entry.GetPtr<T>())
			{
				return Found;
			}
		}
		return nullptr;
	}

	/** 특정 타입의 Fragment가 있는지 확인합니다 */
	template<typename T>
	bool HasFragment() const
	{
		return FindFragment<T>() != nullptr;
	}

public:
	/** 생성할 Instance 클래스 (nullptr이면 UEquipmentInstance 사용) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TSubclassOf<UEquipmentInstance> InstanceClass;

	/** 장비가 차지할 슬롯 태그 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (Categories = "Equipment.Slot"))
	FGameplayTag SlotTag;

	/** QuickBar 슬롯 태그 (무기류만 해당) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (Categories = "QuickBar.Slot"))
	FGameplayTag QuickBarSlotTag;

	/** 장착 시 스폰할 Actor 목록 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TArray<FEquipmentActorToSpawn> ActorsToSpawn;
	
	/** Fragment 목록 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fragments")
	TArray<TInstancedStruct<FEquipmentFragment>> Fragments;
};
