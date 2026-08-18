// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "GameplayTagContainer.h"
#include "Iris/ReplicationState/IrisFastArraySerializer.h"
#include "Coro.h"
#include "EquipmentComponent.generated.h"

class UEquipmentDefinition;
class UEquipmentInstance;
class UItemInstance;
class UEquipmentComponent;
struct FEquipmentList;

DECLARE_LOG_CATEGORY_EXTERN(EquipmentComponentLog, Log, All);

/**
 * 장비 슬롯 변경 델리게이트 (UI 갱신용)
 *
 * 특정 슬롯에 장비가 장착/해제될 때 브로드캐스트됩니다. Instance가 nullptr이면 해제된 것입니다.
 * 서버·클라이언트 모두 슬롯 맵 갱신 경로(AddToSlotMap/RemoveFromSlotMap)에서 호출됩니다.
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnEquipmentSlotChanged, FGameplayTag /*SlotTag*/, UEquipmentInstance* /*Instance*/);

/**
 * 손에 든 메인 장비 변경 델리게이트 (UI용)
 *
 * QuickBarSlotTag를 가진 장비가 활성화(bActive=true)되거나, 활성 상태이던 장비가 완전히
 * 해제되어 아무것도 안 든 상태가 될 때 브로드캐스트됩니다. NewMainItem이 nullptr이면 후자입니다.
 * 서버·클라이언트 모두 활성 상태 반영 경로(SetEntryActiveAuth/ReconcileReplicatedEntryActors 등)에서 호출됩니다.
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMainEquippedItemChanged, UEquipmentInstance* /*NewMainItem*/);

//-----------------------------------------------------------------------------
// FEquipmentEntry
//-----------------------------------------------------------------------------

/**
 * 장착된 장비 항목
 *
 * 리플리케이션 데이터(Definition, SourceItemId)는 Entry에 직접 포함되고,
 * UEquipmentInstance는 클라이언트에서 로컬로 생성됩니다.
 */
USTRUCT(BlueprintType)
struct COMMONGAME_API FEquipmentEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

public:
	void PostReplicatedAdd(const FEquipmentList& InArraySerializer);
	void PostReplicatedChange(const FEquipmentList& InArraySerializer);
	void PreReplicatedRemove(const FEquipmentList& InArraySerializer);

public:
	//-----------------------------------------------------------------------------
	// 리플리케이션 데이터
	//-----------------------------------------------------------------------------

	/** 장비 정의 */
	UPROPERTY()
	TObjectPtr<const UEquipmentDefinition> Definition = nullptr;

	/** 원본 아이템 ID */
	UPROPERTY()
	int32 SourceItemId = INDEX_NONE;

	/**
	 * 액터 스폰 활성 여부
	 *
	 * 데이터(엔트리/슬롯 맵)는 유지한 채 액터만 켜고 끄기 위한 플래그입니다.
	 * 방어구는 항상 true, 손에 드는 메인 장비는 현재 활성인 하나만 true입니다.
	 * 서버가 값을 바꾸면 클라이언트는 PostReplicatedChange에서 액터를 스폰/제거합니다.
	 */
	UPROPERTY()
	bool bActive = false;

	//-----------------------------------------------------------------------------
	// 런타임 데이터 (리플리케이션 안 함)
	//-----------------------------------------------------------------------------

	UPROPERTY(NotReplicated, Transient)
	TObjectPtr<UEquipmentInstance> Instance = nullptr;
};

//-----------------------------------------------------------------------------
// FEquipmentList
//-----------------------------------------------------------------------------

/**
 * 장비 목록을 담는 FastArraySerializer 컨테이너
 */
USTRUCT(BlueprintType)
struct COMMONGAME_API FEquipmentList : public FIrisFastArraySerializer
{
	GENERATED_BODY()

public:
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FEquipmentEntry, FEquipmentList>(Entries, DeltaParms, *this);
	}
	
	/** Entry를 Dirty로 마킹합니다 */
	void MarkEntryDirty(FEquipmentEntry& Entry)
	{
		MarkItemDirty(Entry);
	}

public:
	UPROPERTY()
	TArray<FEquipmentEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UEquipmentComponent> Owner = nullptr;
};

template <>
struct TStructOpsTypeTraits<FEquipmentList> : public TStructOpsTypeTraitsBase2<FEquipmentList>
{
	enum { WithNetDeltaSerializer = true };
};

//-----------------------------------------------------------------------------
// UEquipmentComponent
//-----------------------------------------------------------------------------

/**
 * 장비 관리 컴포넌트
 *
 * Pawn에 붙어서 장착된 장비를 관리합니다.
 */
UCLASS(ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent))
class COMMONGAME_API UEquipmentComponent : public UPawnComponent
{
	GENERATED_BODY()

	friend struct FEquipmentEntry;

public:
	UEquipmentComponent(const FObjectInitializer& ObjectInitializer);

	/** Pawn에서 EquipmentComponent를 찾아 반환합니다 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	static UEquipmentComponent* FindEquipmentComponent(const APawn* Pawn);

	//-----------------------------------------------------------------------------
	// UActorComponent 오버라이드
	//-----------------------------------------------------------------------------

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	//-----------------------------------------------------------------------------
	// 장비 관리 (서버 전용)
	//-----------------------------------------------------------------------------

	TCoroTask<void> HandleActiveEquipChangedAuth(UItemInstance* Instance);

	/** 장비를 해제합니다 (서버 전용) */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	void UnequipItemAuth(FGameplayTag SlotTag);

	/** 아이템을 장착합니다 (서버 전용, 코루틴) */
	TCoroTask<UEquipmentInstance*> EquipItemAuthCoroutine(UItemInstance* Item);

	//-----------------------------------------------------------------------------
	// 장비 조회
	//-----------------------------------------------------------------------------

	/** 슬롯에 장착된 장비를 반환합니다 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	UEquipmentInstance* GetEquipmentInSlot(FGameplayTag SlotTag) const;

	/** 슬롯에 장착된 스폰된 Actor를 반환합니다 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	AActor* GetEquipmentInstance(FGameplayTag SlotTag) const;

	/** 슬롯 장비 변경 델리게이트 (UI 갱신용, 서버/클라이언트 모두) */
	FOnEquipmentSlotChanged OnEquipmentSlotChanged;

	/** 손에 든 메인 장비 변경 델리게이트 (UI 갱신용, 서버/클라이언트 모두) */
	FOnMainEquippedItemChanged OnMainEquippedItemChanged;

	/** 현재 활성화된(손에 든) 메인 장비를 반환합니다. 서버/클라이언트 모두 EquipmentSlots에서 직접 조회합니다 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	UEquipmentInstance* GetActiveMainEquippedItem() const;

	//-----------------------------------------------------------------------------
	// QuickBar
	//-----------------------------------------------------------------------------
	
	bool CanSetActiveSlot(FGameplayTag SlotTag) const;

	UFUNCTION(Server, Reliable)
	void SetActiveSlotServer(FGameplayTag SlotTag);
	
private:
	/** EquipmentInstance를 생성합니다 */
	UEquipmentInstance* CreateEquipmentInstance(const UEquipmentDefinition* Definition, int32 SourceItemId) const;

	/** 실제 장비 장착 로직 (번들 로딩 완료 상태에서 호출) */
	UEquipmentInstance* EquipItemAuthInternal(UItemInstance* Item);

	/** 손에 드는 메인 장비를 전환합니다 (이전 메인 액터 해제 → 새 메인 스폰, 데이터는 유지, 서버 전용) */
	void SetMainEquippedAuth(UEquipmentInstance* NewMain);

	/** 엔트리의 액터 활성 상태를 바꾸고 복제합니다 (데이터 유지, 서버 전용) */
	void SetEntryActiveAuth(FEquipmentEntry& Entry, bool bNewActive);

	/** 복제로 전달된 bActive 변경에 맞춰 클라이언트에서 액터를 스폰/제거합니다 */
	void ReconcileReplicatedEntryActors(FEquipmentEntry* Entry);

	/** Instance로 Entry를 찾습니다 (없으면 nullptr) */
	FEquipmentEntry* FindEntryByInstance(const UEquipmentInstance* Instance);

	/** QuickBar 슬롯 태그로 장착된 장비를 찾습니다 (없으면 nullptr) */
	UEquipmentInstance* FindEquipmentByQuickBarSlot(FGameplayTag QuickBarSlotTag) const;

	/** 복제된 Entry 초기화를 시작합니다 (기존 태스크 취소 후 새 태스크 시작) */
	void StartReplicatedEquipmentInit(FEquipmentEntry* Entry);

	/** 복제된 Entry에 대해 로컬 EquipmentInstance를 생성하고 초기화합니다 (클라이언트 전용) */
	TCoroTask<void> InitializeReplicatedEquipmentCoroutine(FEquipmentEntry* Entry);

	/** SourceItemId로 Entry를 찾습니다 (비동기 대기 후 재조회용, 없으면 nullptr) */
	FEquipmentEntry* FindEntryBySourceItemId(int32 SourceItemId);

	/** 장비를 슬롯 맵에 등록합니다 */
	void AddToSlotMap(UEquipmentInstance* Instance);

	/** 장비를 슬롯 맵에서 제거합니다 */
	void RemoveFromSlotMap(UEquipmentInstance* Instance);

protected:
	/** 장비 목록 (네트워크 리플리케이션용) */
	UPROPERTY(Replicated)
	FEquipmentList EquipmentList;

	/**
	 * 슬롯별 장비 맵
	 *
	 * 블루프린트에서 사용할 슬롯 태그를 Key로 등록합니다.
	 * 장착 시 해당 슬롯이 없으면 장착이 실패합니다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment", meta = (Categories = "Equipment.Slot"))
	TMap<FGameplayTag, TObjectPtr<UEquipmentInstance>> EquipmentSlots;


	/** 현재 장착된 메인 장비 (서버 전용, 손에 들고 있는 것) */
	UPROPERTY()
	TObjectPtr<UEquipmentInstance> MainEquippedItem = nullptr;

private:
	/** 슬롯별 진행 중인 초기화 태스크 (새 요청 시 취소용) */
	TMap<FGameplayTag, TCoroTask<void>> PendingInitTasks;
};
