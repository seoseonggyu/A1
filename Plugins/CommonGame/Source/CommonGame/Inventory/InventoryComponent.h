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

/**
 * 그리드 변경 종류
 *
 * UI가 ItemId 기준으로 위젯을 생성/갱신/제거할 수 있도록 변경 유형을 구분합니다.
 */
UENUM(BlueprintType)
enum class EInventoryGridChangeType : uint8
{
	/** 새로 배치됨 → 위젯 생성 */
	Added,
	/** 스택/상태 변경 → 위젯 갱신 */
	Changed,
	/** 위치 이동 → 위젯 위치 이동 */
	Moved,
	/** 제거됨 → 위젯 제거 (Instance는 아직 유효) */
	Removed
};

/**
 * 2D 그리드 슬롯 변경 델리게이트 (UI용)
 *
 * ChangeType으로 배치/변경/이동/제거를 구분하며, ItemId로 위젯을 매핑합니다.
 * Removed일 때도 Instance/ItemId는 유효한 값으로 전달됩니다.
 */
DECLARE_MULTICAST_DELEGATE_FiveParams(FOnInventoryGridChanged, EInventoryGridChangeType /*ChangeType*/, int32 /*ItemId*/, UItemInstance* /*Instance*/, const FIntPoint& /*SlotPos*/, int32 /*StackCount*/);

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

	/** 2D 그리드 앵커 위치(좌상단 셀). (-1,-1) 이면 아직 배치되지 않은 상태 */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	FIntPoint SlotPosition = FIntPoint(-1, -1);

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

	//-----------------------------------------------------------------------------
	// 2D 그리드
	//-----------------------------------------------------------------------------

	/** 그리드 크기(칸 수)를 반환합니다 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Grid")
	FIntPoint GetGridSize() const { return GridSize; }
	
	/** 좌표가 그리드 범위 내인지 확인합니다 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Grid")
	bool IsValidSlot(FIntPoint SlotPos) const;

	/** 해당 셀을 점유 중인 아이템을 반환합니다 (없으면 nullptr) */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Grid")
	UItemInstance* GetItemAtSlot(FIntPoint SlotPos) const;

	/** 아이템의 그리드 앵커 위치를 반환합니다 (미배치 시 (-1,-1)) */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Grid")
	FIntPoint GetSlotPosition(const UItemInstance* Instance) const;

	/**
	 * 지정 영역(Pos ~ Pos+Size)이 모두 비어 있는지 확인합니다.
	 * @param IgnoreItemId 겹침 검사에서 제외할 아이템 ID (이동 중인 자기 자신)
	 */
	bool IsRegionEmpty(FIntPoint SlotPos, FIntPoint Size, int32 IgnoreItemId = INDEX_NONE) const;

	/** 지정 크기의 아이템을 놓을 수 있는 첫 번째 빈 위치를 찾습니다 (서버 배치용) */
	bool FindEmptySlot(FIntPoint Size, FIntPoint& OutSlotPos) const;

	/**
	 * 아이템을 특정 위치에 놓을 수 있는지 판정합니다 (UI 초록/빨강 미리보기).
	 * 자기 자신과의 겹침은 허용합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Grid")
	bool CanPlaceItemAt(int32 ItemId, FIntPoint SlotPos) const;

	/** 아이템을 새 그리드 위치로 이동합니다 (서버 전용) */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory|Grid")
	bool MoveItemAuth(int32 ItemId, FIntPoint NewSlotPos);

	/** 아이템 이동을 요청합니다 (클라이언트 → 서버 RPC) */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Inventory|Grid")
	void MoveItemServer(int32 ItemId, FIntPoint NewSlotPos);

	/**
	 * 아이템을 그리드에서 미배치 상태로 만듭니다 (서버 전용).
	 *
	 * 인벤토리 목록/ItemId 조회는 그대로 유지한 채 그리드 표시에서만 제외합니다.
	 * QuickBar 등록·장비 장착 등 "아이템의 실체는 유지하되 그리드 칸을 비워야 하는" 경우에 사용합니다.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory|Grid")
	bool UnplaceItemAuth(int32 ItemId);

	/** 미배치 상태인 아이템을 빈 칸을 찾아 그리드에 다시 배치합니다 (서버 전용) */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory|Grid")
	bool PlaceAtEmptySlotAuth(int32 ItemId);

	/** 아이템이 차지하는 그리드 크기를 반환합니다 (Definition->SlotCount, 최소 1x1) */
	static FIntPoint GetSizeFromDefinition(const UItemDefinition* InDefinition);

	/** 그리드 슬롯 변경 델리게이트 (UI 갱신용) */
	FOnInventoryGridChanged OnInventoryGridChanged;
	
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
	 * 실제 아이템 인스턴스 생성 로직 (번들 로딩 완료 상태에서 호출)
	 *
	 * Entry의 Definition/ItemId/StackCount/SlotPosition은 호출 전에 이미 채워져 있어야 합니다.
	 * @param ReservedIndex 미리 예약된 Entry 인덱스
	 */
	UItemInstance* AddItemAuthInternal(const UItemDefinition* Definition, int32 Count, FIntPoint SlotPos, int32 ReservedIndex);

protected:
	/** 새로운 아이템 ID를 생성합니다 (서버 전용, 서버 전체에서 고유) */
	static int32 GenerateItemId();

protected:
	/** 인벤토리 2D 그리드 크기(칸 수) */
	UPROPERTY(Replicated)
	FIntPoint GridSize = FIntPoint(10, 8);

	/** 인벤토리 목록 */
	UPROPERTY(Replicated)
	FInventoryList InventoryList;

	/** 아이템 NetState 목록 (최상위 FastArraySerializer로 델타 복제) */
	UPROPERTY(Replicated)
	FItemNetStateList ItemNetStates;

	/** 초기 아이템 목록 */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Initial Items")
	TArray<FInitialItemEntry> InitialItems;

	/** 아이템 ID -> Instance 맵 */
	TMap<int32, TObjectPtr<UItemInstance>> ItemMap;

private:
	/** 초기 아이템 지급 완료 여부 */
	bool bInitialItemsGiven = false;
};
