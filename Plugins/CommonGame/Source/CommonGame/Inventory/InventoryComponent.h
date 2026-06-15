// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ControllerComponent.h"
#include "Inventory/ItemInstance.h"
#include "Inventory/Fragment/NetState/ItemFragmentNetStateList.h"
#include "Coro.h"
#include "InventoryComponent.generated.h"

struct FInventoryList;
class UItemDefinition;
class UExperienceDefinition;
class UInventoryComponent;

DECLARE_LOG_CATEGORY_EXTERN(InventoryComponentLog, Log, All);

/** 아이템 생성 및 초기화 완료 델리게이트 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryItemReady, int32 /*ItemId*/, UItemInstance* /*Instance*/);

//-----------------------------------------------------------------------------
// FInitialItemEntry
//-----------------------------------------------------------------------------

/**
 * 초기 아이템 설정 항목
 *
 * 게임 시작 시 자동으로 추가될 아이템을 정의합니다.
 */
USTRUCT(BlueprintType)
struct COMMONGAME_API FInitialItemEntry
{
	GENERATED_BODY()

public:
	bool IsValid() const { return ItemDefinition != nullptr && Count > 0; }

public:
	/** 추가할 아이템 정의 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TObjectPtr<const UItemDefinition> ItemDefinition;

	/** 추가할 개수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "1"))
	int32 Count = 1;

	/** QuickBar에도 추가할지 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	bool bAddToQuickBar = false;
};

//-----------------------------------------------------------------------------
// FInventoryEntry
//-----------------------------------------------------------------------------

/**
 * 인벤토리 항목
 *
 * 아이템 데이터와 런타임 인스턴스를 담습니다.
 * 리플리케이션 데이터(Definition, ItemId)는 Entry에 직접 포함되고,
 * UItemInstance는 클라이언트에서 로컬로 생성됩니다.
 * NetState는 중첩 FastArraySerializer를 피하기 위해 UInventoryComponent::ItemNetStates에서 관리합니다.
 */
USTRUCT(BlueprintType)
struct COMMONGAME_API FInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

public:
	void PostReplicatedAdd(const FInventoryList& InArraySerializer);
	void PostReplicatedChange(const FInventoryList& InArraySerializer);
	void PreReplicatedRemove(const FInventoryList& InArraySerializer);

	void NotifyCreated(UInventoryComponent* Owner) const;
	void NotifyChanged(UInventoryComponent* Owner) const;
	void NotifyRemoved(UInventoryComponent* Owner) const;

public:
	//-----------------------------------------------------------------------------
	// 리플리케이션 데이터
	//-----------------------------------------------------------------------------

	/** 아이템 정의 */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<const UItemDefinition> Definition = nullptr;

	/** 고유 아이템 ID */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 ItemId = INDEX_NONE;

	/** 스택 수량 */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 StackCount = 1;

	//-----------------------------------------------------------------------------
	// 런타임 데이터 (리플리케이션 안 함)
	//-----------------------------------------------------------------------------

	/** 아이템 인스턴스 (서버: 직접 생성, 클라이언트: PostReplicatedAdd에서 로컬 생성) */
	UPROPERTY(NotReplicated, Transient, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UItemInstance> Instance = nullptr;
};

//-----------------------------------------------------------------------------
// FInventoryList
//-----------------------------------------------------------------------------

/**
 * 인벤토리 항목 배열을 담는 FastArraySerializer 컨테이너
 *
 * 네트워크 델타 직렬화를 지원합니다.
 */
USTRUCT(BlueprintType)
struct COMMONGAME_API FInventoryList : public FIrisFastArraySerializer
{
	GENERATED_BODY()

public:
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryEntry, FInventoryList>(Entries, DeltaParms, *this);
	}

	/** Entry를 Dirty로 마킹합니다 */
	void MarkEntryDirty(FInventoryEntry& Entry)
	{
		MarkItemDirty(Entry);
	}

public:
	/** 인벤토리 항목 배열 */
	UPROPERTY()
	TArray<FInventoryEntry> Entries;

	/** 소유 컴포넌트 (복제되지 않음) */
	UPROPERTY(NotReplicated)
	TObjectPtr<UInventoryComponent> Owner = nullptr;
};

template<>
struct TStructOpsTypeTraits<FInventoryList> : public TStructOpsTypeTraitsBase2<FInventoryList>
{
	enum { WithNetDeltaSerializer = true };
};

//-----------------------------------------------------------------------------
// UInventoryComponent
//-----------------------------------------------------------------------------

/**
 * 플레이어 인벤토리 컴포넌트
 *
 * PlayerController에 붙어서 아이템을 관리합니다.
 * 네트워크 복제를 지원하며, Owner에게만 복제됩니다.
 * Experience 로드 완료 시 초기 아이템을 자동으로 추가합니다.
 */
UCLASS(ClassGroup = (Inventory), meta = (BlueprintSpawnableComponent))
class COMMONGAME_API UInventoryComponent : public UControllerComponent
{
	GENERATED_BODY()

	friend struct FInventoryEntry;
	friend struct FItemNetStateEntry;
	friend class UItemInstance;

public:
	UInventoryComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static UInventoryComponent* FindInventoryComponent(const APawn* Pawn);
	static UInventoryComponent* FindInventoryComponent(const AController* Controller);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	//-----------------------------------------------------------------------------
	// 아이템 관리 (서버 전용)
	//-----------------------------------------------------------------------------

	/** 아이템을 추가합니다 (서버 전용, 코루틴) */
	TCoroTask<UItemInstance*> AddItemAuthCoroutine(const UItemDefinition* Definition, int32 Count = 1);

	/** 아이템을 제거합니다 (서버 전용) */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	bool RemoveItemAuth(UItemInstance* Instance);

	/** 스택 수량을 변경합니다 (서버 전용) */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	bool ModifyStackCountAuth(UItemInstance* Instance, int32 NewCount);

	/** TagStat 값을 변경합니다 (클라이언트 → 서버 RPC) */
	UFUNCTION(Server, Reliable)
	void ModifyTagStatServer(int32 ItemId, FGameplayTag StatTag, float NewValue);

	//-----------------------------------------------------------------------------
	// 초기 아이템 (서버 전용)
	//-----------------------------------------------------------------------------

	/** 초기 아이템을 지급합니다 (서버 전용, Pawn Possess 시 호출) */
	UFUNCTION()
	void GiveInitialItemsAuth(APawn* OldPawn, APawn* NewPawn);

	//-----------------------------------------------------------------------------
	// 아이템 조회
	//-----------------------------------------------------------------------------

	/** 특정 슬롯의 아이템을 찾습니다 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UItemInstance* FindItemAtSlot(int32 SlotIndex) const;

	/** ID로 아이템을 찾습니다 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UItemInstance* FindItemById(int32 ItemId) const;

	/** 모든 아이템을 반환합니다 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void GetAllItems(TArray<UItemInstance*>& OutItems) const;

	/** 아이템 개수를 반환합니다 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemCount() const { return InventoryList.Entries.Num(); }

	/** 최대 슬롯 수를 반환합니다 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetMaxSlots() const { return MaxSlots; }

	/** 슬롯이 가득 찼는지 확인합니다 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool IsSlotFull() const { return GetItemCount() >= MaxSlots; }

	/** 아이템의 Entry를 찾습니다 */
	FInventoryEntry* FindEntry(const UItemInstance* Instance);
	const FInventoryEntry* FindEntry(const UItemInstance* Instance) const;

	/** 아이템 생성 및 초기화 완료 델리게이트 (서버/클라이언트 모두) */
	FOnInventoryItemReady OnItemReady;

	/** 특정 Fragment를 가진 아이템을 찾습니다 */
	template<typename T>
	UItemInstance* FindItemWithFragment() const
	{
		for (const FInventoryEntry& Entry : InventoryList.Entries)
		{
			if (Entry.Instance && Entry.Instance->FindFragment<T>())
			{
				return Entry.Instance;
			}
		}
		return nullptr;
	}

	/** 특정 Fragment를 가진 모든 아이템을 찾습니다 */
	template<typename T>
	TArray<UItemInstance*> FindAllItemsWithFragment() const
	{
		TArray<UItemInstance*> Result;
		for (const FInventoryEntry& Entry : InventoryList.Entries)
		{
			if (Entry.Instance && Entry.Instance->FindFragment<T>())
			{
				Result.Add(Entry.Instance);
			}
		}
		return Result;
	}

	/** 복제된 Entry에 대해 로컬 ItemInstance를 생성하고 초기화합니다 (클라이언트 전용) */
	TCoroTask<void> InitializeReplicatedItemCoroutine(FInventoryEntry* Entry);

private:
	/**
	 * 실제 아이템 추가 로직 (번들 로딩 완료 상태에서 호출)
	 * @param ReservedIndex 미리 예약된 슬롯 인덱스
	 */
	UItemInstance* AddItemAuthInternal(const UItemDefinition* Definition, int32 Count, int32 ReservedIndex);

protected:
	/** 새로운 아이템 ID를 생성합니다 (서버 전용, 서버 전체에서 고유) */
	static int32 GenerateItemId();

protected:
	/** 인벤토리 목록 */
	UPROPERTY(Replicated)
	FInventoryList InventoryList;

	/** 아이템 NetState 목록 (최상위 FastArraySerializer로 델타 복제) */
	UPROPERTY(Replicated)
	FItemNetStateList ItemNetStates;

	/** 최대 슬롯 수 */
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 MaxSlots = 36;

	/** 초기 아이템 목록 */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Initial Items")
	TArray<FInitialItemEntry> InitialItems;

	/** 아이템 ID -> Instance 맵 */
	TMap<int32, TObjectPtr<UItemInstance>> ItemMap;

private:
	/** 초기 아이템 지급 완료 여부 */
	bool bInitialItemsGiven = false;
};
