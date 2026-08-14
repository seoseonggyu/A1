// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "Equipment/EquipmentDefinition.h"
#include "EquipmentInstance.generated.h"

class UEquipmentDefinition;
class UItemInstance;
class ACommonCharacter;

/**
 * 런타임 장비 인스턴스
 *
 * 장착된 아이템의 런타임 상태를 관리합니다.
 * 스폰된 Actor를 추적하고, Fragment 콜백을 호출합니다.
 */
UCLASS(BlueprintType)
class COMMONGAME_API UEquipmentInstance : public UObject
{
	GENERATED_BODY()

	friend class UEquipmentComponent;
	friend struct FEquipmentEntry;

public:
	//-----------------------------------------------------------------------------
	// 접근자
	//-----------------------------------------------------------------------------

	/** 장비 정의를 반환합니다 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	const UEquipmentDefinition* GetDefinition() const { return Definition; }

	/** 스폰된 Actor 목록을 반환합니다 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	const TArray<AActor*>& GetSpawnedActors() const { return SpawnedActors; }

	/** 원본 ItemId를 반환합니다 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	int32 GetSourceItemId() const { return SourceItemId; }

	/** 원본 ItemInstance를 반환합니다 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	UItemInstance* GetSourceItemInstance() const;

	/** 소유 Character를 반환합니다 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	ACommonCharacter* GetOwningCharacter() const;

	/** 서버 권한이 있는지 확인합니다 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool HasAuthority() const;

	/** 로컬 플레이어가 소유하는지 확인합니다 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool IsLocallyControlled() const;

	/** Equipment 슬롯 태그를 반환합니다 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	FGameplayTag GetEquipmentSlotTag() const { return Definition ? Definition->SlotTag : FGameplayTag(); }

	/** QuickBar 슬롯 태그를 반환합니다 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	FGameplayTag GetQuickBarSlotTag() const { return Definition ? Definition->QuickBarSlotTag : FGameplayTag(); }

	//-----------------------------------------------------------------------------
	// Fragment API
	//-----------------------------------------------------------------------------

	/** Fragment를 검색합니다 */
	template<typename T>
	T* FindFragment()
	{
		for (TInstancedStruct<FEquipmentFragment>& Entry : Fragments)
		{
			if (T* Found = Entry.GetMutablePtr<T>())
			{
				return Found;
			}
		}
		return nullptr;
	}

	/** Fragment를 검색합니다 (const) */
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
	/**
	 * 장비를 활성화합니다 (액터 스폰 + OnEquipped).
	 *
	 * 이미 활성 상태면 아무것도 하지 않습니다(멱등). 메인 장비 전환 시 데이터는 유지한 채
	 * 액터/Fragment만 켜고 끄기 위해 사용합니다.
	 */
	void ActivateEquipment();

	/** 장비를 비활성화합니다 (OnUnequipped + 액터 제거). 이미 비활성이면 무시(멱등). */
	void DeactivateEquipment();

	/** 현재 액터가 스폰되어 활성 상태인지 반환합니다 */
	bool IsEquipmentActive() const { return bEquipmentActive; }

protected:
	/** 장착 시 호출됩니다 (Fragment 복사 및 콜백, 서버+클라이언트) */
	virtual void OnEquipped();

	/** 해제 시 호출됩니다 (Fragment 콜백 및 정리, 서버+클라이언트) */
	virtual void OnUnequipped();

	/** 장비 Actor들을 스폰합니다 (클라이언트 전용) */
	void SpawnEquipmentActors();

	/** 장비 Actor들을 제거합니다 (클라이언트 전용) */
	void DestroyEquipmentActors();

private:
	/** 단일 Actor를 스폰합니다 */
	AActor* SpawnActor(const FEquipmentActorToSpawn& ActorToSpawn) const;

protected:
	/** 장비 정의 */
	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<const UEquipmentDefinition> Definition = nullptr;

	/** 원본 아이템 ID (Inventory의 ItemInstance.ItemId) */
	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	int32 SourceItemId = INDEX_NONE;

	/** 스폰된 Actor 목록 */
	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TArray<AActor*> SpawnedActors;

	/** Fragment 인스턴스 목록 (Definition에서 복사됨) */
	TArray<TInstancedStruct<FEquipmentFragment>> Fragments;

private:
	/** 액터 스폰/Fragment 적용 여부 (중복 Activate/Deactivate 방지, 비복제 런타임 상태) */
	bool bEquipmentActive = false;
};
