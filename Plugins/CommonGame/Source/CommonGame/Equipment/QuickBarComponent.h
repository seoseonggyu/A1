// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ControllerComponent.h"
#include "GameplayTagContainer.h"
#include "Iris/ReplicationState/IrisFastArraySerializer.h"
#include "Coro.h"
#include "QuickBarComponent.generated.h"

class UItemInstance;
class UEquipmentInstance;
class UEquipmentComponent;
class UInventoryComponent;
struct FQuickBarList;

DECLARE_LOG_CATEGORY_EXTERN(QuickBarComponentLog, Log, All);

//-----------------------------------------------------------------------------
// FQuickBarActiveSlot
//-----------------------------------------------------------------------------

/**
 * 현재 활성 슬롯 정보
 */
USTRUCT(BlueprintType)
struct COMMONGAME_API FQuickBarActiveSlot
{
	GENERATED_BODY()

public:
	bool IsValid() const { return SlotTag.IsValid(); }
	bool operator==(const FQuickBarActiveSlot& Other) const { return SlotTag == Other.SlotTag && Index == Other.Index; }
	bool operator!=(const FQuickBarActiveSlot& Other) const { return !(*this == Other); }

public:
	/** 활성 슬롯 태그 */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag SlotTag;

	/** 슬롯 내 인덱스 */
	UPROPERTY(BlueprintReadOnly)
	int32 Index = 0;
};

//-----------------------------------------------------------------------------
// FQuickBarSlotData
//-----------------------------------------------------------------------------

/**
 * 슬롯에 장착된 아이템 데이터
 *
 * 블루프린트에서 MaxCount를 설정하고,
 * 런타임에 Items 배열이 채워집니다.
 */
USTRUCT(BlueprintType)
struct COMMONGAME_API FQuickBarSlotData
{
	GENERATED_BODY()

public:
	/** 실제 아이템 개수를 반환합니다 (nullptr 제외) */
	int32 Num() const
	{
		int32 Count = 0;
		for (const auto& Item : Items)
		{
			if (Item) ++Count;
		}
		return Count;
	}

	bool IsEmpty() const { return Num() == 0; }
	bool IsFull() const { return Num() >= MaxCount; }

	/** 첫 번째 빈 슬롯 인덱스를 반환합니다 */
	int32 FindEmptyIndex() const
	{
		for (int32 i = 0; i < Items.Num(); ++i)
		{
			if (!Items[i])
				return i;
		}

		return INDEX_NONE;
	}

public:
	/** 슬롯 최대 개수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxCount = 1;

	/** 슬롯에 장착된 아이템들 */
	UPROPERTY(BlueprintReadOnly)
	TArray<TObjectPtr<UItemInstance>> Items;
};

//-----------------------------------------------------------------------------
// FQuickBarEntry
//-----------------------------------------------------------------------------

/**
 * QuickBar 항목
 *
 * 아이템 ID와 슬롯 정보를 담습니다.
 * UItemInstance 포인터는 복제되지 않으므로 ItemId로 참조합니다.
 */
USTRUCT(BlueprintType)
struct COMMONGAME_API FQuickBarEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

public:
	void PostReplicatedAdd(const FQuickBarList& InArraySerializer);
	void PostReplicatedChange(const FQuickBarList& InArraySerializer);
	void PreReplicatedRemove(const FQuickBarList& InArraySerializer);

public:
	/** 아이템 ID (InventoryComponent에서 Instance를 조회할 때 사용) */
	UPROPERTY(BlueprintReadOnly, Category = "QuickBar")
	int32 ItemId = INDEX_NONE;

	/** 슬롯 내 인덱스 */
	UPROPERTY(BlueprintReadOnly, Category = "QuickBar")
	int32 SlotIndex = 0;
};

//-----------------------------------------------------------------------------
// FQuickBarList
//-----------------------------------------------------------------------------

/**
 * QuickBar 항목 배열을 담는 FastArraySerializer 컨테이너
 *
 * 네트워크 델타 직렬화를 지원합니다.
 */
USTRUCT(BlueprintType)
struct COMMONGAME_API FQuickBarList : public FIrisFastArraySerializer
{
	GENERATED_BODY()

public:
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FQuickBarEntry, FQuickBarList>(Entries, DeltaParms, *this);
	}

	/** Entry를 Dirty로 마킹합니다 */
	void MarkEntryDirty(FQuickBarEntry& Entry)
	{
		MarkItemDirty(Entry);
	}

public:
	/** QuickBar 항목 배열 */
	UPROPERTY()
	TArray<FQuickBarEntry> Entries;

	/** 소유 컴포넌트 (복제되지 않음) */
	UPROPERTY(NotReplicated)
	TObjectPtr<UQuickBarComponent> Owner = nullptr;
};

template<>
struct TStructOpsTypeTraits<FQuickBarList> : public TStructOpsTypeTraitsBase2<FQuickBarList>
{
	enum { WithNetDeltaSerializer = true };
};

//-----------------------------------------------------------------------------
// UQuickBarComponent
//-----------------------------------------------------------------------------

/**
 * 퀵바 컴포넌트
 *
 * PlayerController에 붙어서 퀵바 슬롯을 관리합니다.
 * 무기 교체 등 활성 슬롯 변경을 처리합니다.
 */
UCLASS(ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent))
class COMMONGAME_API UQuickBarComponent : public UControllerComponent
{
	GENERATED_BODY()

	friend struct FQuickBarEntry;

public:
	UQuickBarComponent(const FObjectInitializer& ObjectInitializer);

	//-----------------------------------------------------------------------------
	// UActorComponent 오버라이드
	//-----------------------------------------------------------------------------

	virtual void InitializeComponent() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//-----------------------------------------------------------------------------
	// 슬롯 관리 (서버 전용)
	//-----------------------------------------------------------------------------

	/** 아이템을 슬롯에 추가합니다 (서버 전용) */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "QuickBar")
	bool AddItemToSlotAuth(UItemInstance* Item);

	/** 아이템을 슬롯에서 제거합니다 (서버 전용) */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "QuickBar")
	bool RemoveItemFromSlotAuth(UItemInstance* Item);

	/** 활성 슬롯을 변경합니다 (서버 전용) */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "QuickBar")
	void SetActiveSlotAuth(FGameplayTag SlotTag, int32 Index = 0);

	//-----------------------------------------------------------------------------
	// 슬롯 조회
	//-----------------------------------------------------------------------------

	/** 활성 슬롯 정보를 반환합니다 */
	UFUNCTION(BlueprintCallable, Category = "QuickBar")
	const FQuickBarActiveSlot& GetActiveSlot() const { return ActiveSlot; }

	/** 활성 슬롯의 아이템을 반환합니다 */
	UFUNCTION(BlueprintCallable, Category = "QuickBar")
	UItemInstance* GetActiveItem() const;

	/** 특정 슬롯의 아이템들을 반환합니다 */
	const FQuickBarSlotData* GetSlotData(FGameplayTag SlotTag) const;

	/** 모든 슬롯 데이터를 반환합니다 */
	const TMap<FGameplayTag, FQuickBarSlotData>& GetQuickBarSlots() const { return QuickBarSlots; }

	/** 슬롯이 등록되어 있는지 확인합니다 */
	UFUNCTION(BlueprintCallable, Category = "QuickBar")
	bool IsSlotRegistered(FGameplayTag SlotTag) const { return QuickBarSlots.Contains(SlotTag); }

	/** 슬롯이 가득 찼는지 확인합니다 */
	UFUNCTION(BlueprintCallable, Category = "QuickBar")
	bool IsSlotFull(FGameplayTag SlotTag) const;

	/** 아이템이 어느 슬롯에든 등록되어 있는지 확인합니다 */
	UFUNCTION(BlueprintCallable, Category = "QuickBar")
	bool IsItemRegistered(UItemInstance* Item) const;

	/** 슬롯 변경 요청이 유효한지 검사합니다 */
	UFUNCTION(BlueprintCallable, Category = "QuickBar")
	bool CanSetActiveSlot(FGameplayTag SlotTag, int32 Index) const;

	/** EquipmentComponent를 가져옵니다 (캐싱) */
	UFUNCTION(BlueprintCallable, Category = "QuickBar")
	UEquipmentComponent* GetEquipmentComponent() const;

	//-----------------------------------------------------------------------------
    // 슬롯 변경 요청 (클라이언트 → 서버)
    //-----------------------------------------------------------------------------
	
	/** 활성 슬롯 변경 서버 RPC (CanSetActiveSlot 검증 완료 후 호출) */
	UFUNCTION(Server, Reliable)
	void SetActiveSlotServer(FGameplayTag SlotTag, int32 Index);

private:
	/** 클라이언트에서 아이템 초기화 완료 후 SlotMap에 등록합니다 */
	TCoroTask<void> WaitForItemInitializedCoroutine(int32 ItemId, int32 SlotIndex);

	/** 슬롯 맵에 아이템을 추가합니다 */
	void AddToSlotMap(UItemInstance* Item, FGameplayTag SlotTag, int32 SlotIndex);

	/** 슬롯 맵에서 아이템을 제거합니다 */
	void RemoveFromSlotMap(UItemInstance* Item, FGameplayTag SlotTag, int32 SlotIndex);

	/** Entry 인덱스를 찾습니다 */
	int32 FindEntryIndex(UItemInstance* Item) const;

	/** 활성 슬롯 변경 시 장비 교체를 처리합니다 */
	void HandleActiveSlotChanged(const FQuickBarActiveSlot& OldSlot, const FQuickBarActiveSlot& NewSlot);

	/** 아이템을 장착합니다 */
	TCoroTask<void> EquipItemCoroutine(UItemInstance* Item);

	/** 아이템을 해제합니다 */
	void UnequipItem(const UItemInstance* Item);

protected:
	/** QuickBar 목록 (네트워크 리플리케이션용) */
	UPROPERTY(Replicated)
	FQuickBarList QuickBarList;

	/** 활성 슬롯 */
	UPROPERTY(Replicated)
	FQuickBarActiveSlot ActiveSlot;

	/**
	 * 슬롯별 데이터 맵
	 *
	 * 블루프린트에서 사용할 슬롯 태그를 Key로 등록합니다.
	 * 등록되지 않은 슬롯의 아이템은 추가할 수 없습니다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "QuickBar", meta = (Categories = "QuickBar.Slot"))
	TMap<FGameplayTag, FQuickBarSlotData> QuickBarSlots;

	/** 현재 장착된 장비 (서버 전용, EquipmentComponent에서 관리) */
	UPROPERTY()
	TObjectPtr<UEquipmentInstance> EquippedItem = nullptr;

	/** 캐싱된 EquipmentComponent */
	mutable TObjectPtr<UEquipmentComponent> CachedEquipmentComponent = nullptr;
	
};
