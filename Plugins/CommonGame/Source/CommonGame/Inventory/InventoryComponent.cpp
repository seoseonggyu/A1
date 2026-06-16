// Copyright Epic Games, Inc. All Rights Reserved.

#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemDefinition.h"
#include "Equipment/QuickBarComponent.h"
#include "Coroutine/CommonAssetAwaiters.h"
#include "Net/UnrealNetwork.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryComponent)

DEFINE_LOG_CATEGORY(InventoryComponentLog);

/** 다음 아이템 ID (서버 전체에서 고유) */
int32 UInventoryComponent::GenerateItemId()
{
	static int32 NextItemId = 1;

	return NextItemId++;
}

//-----------------------------------------------------------------------------
// FInventoryEntry
//-----------------------------------------------------------------------------

void FInventoryEntry::PostReplicatedAdd(const FInventoryList& InArraySerializer)
{
	// 빈 Entry는 무시 (PostReplicatedChange에서 데이터가 채워지면 처리)
	if (!Definition || ItemId == INDEX_NONE)
	{
		return;
	}

	if (InArraySerializer.Owner)
	{
		// const_cast 필요: PostReplicatedAdd는 const this를 제공하지만 Instance 설정이 필요함
		InArraySerializer.Owner->InitializeReplicatedItemCoroutine(const_cast<FInventoryEntry*>(this));
	}
}

void FInventoryEntry::PostReplicatedChange(const FInventoryList& InArraySerializer)
{
	// 빈 Entry가 데이터로 채워졌을 때 초기화 (PostReplicatedAdd에서 빈 상태로 왔던 경우)
	if (ItemId != INDEX_NONE && Definition && !Instance && InArraySerializer.Owner)
	{
		InArraySerializer.Owner->InitializeReplicatedItemCoroutine(const_cast<FInventoryEntry*>(this));
		return;
	}

	NotifyChanged(InArraySerializer.Owner);
}

void FInventoryEntry::PreReplicatedRemove(const FInventoryList& InArraySerializer)
{
	NotifyRemoved(InArraySerializer.Owner);

	// ItemMap에서 제거
	if (InArraySerializer.Owner && Instance)
	{
		InArraySerializer.Owner->ItemMap.Remove(Instance->ItemId);
	}
}

void FInventoryEntry::NotifyCreated(UInventoryComponent* Owner) const
{
	if (Instance)
	{
		Instance->OnAdded(Owner, StackCount);
	}
}

void FInventoryEntry::NotifyChanged(UInventoryComponent* Owner) const
{
	if (Instance)
	{
		Instance->OnChanged(Owner, StackCount);
	}
}

void FInventoryEntry::NotifyRemoved(UInventoryComponent* Owner) const
{
	if (Instance)
	{
		Instance->OnRemoved(Owner);
	}
}

//-----------------------------------------------------------------------------
// UInventoryComponent
//-----------------------------------------------------------------------------

UInventoryComponent::UInventoryComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	InventoryList.Owner = this;
	ItemNetStates.Owner = this;
}

UInventoryComponent* UInventoryComponent::FindInventoryComponent(const APawn* Pawn)
{
	return Pawn ? FindInventoryComponent(Pawn->GetController()) : nullptr;
}

UInventoryComponent* UInventoryComponent::FindInventoryComponent(const AController* Controller)
{
	return Controller ? Controller->FindComponentByClass<UInventoryComponent>() : nullptr;
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	Params.Condition = COND_OwnerOnly;

	DOREPLIFETIME_WITH_PARAMS_FAST(UInventoryComponent, InventoryList, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UInventoryComponent, ItemNetStates, Params);
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// 서버에서만 Pawn Possess 시 초기 아이템 지급
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (AController* Controller = Cast<AController>(GetOwner()))
		{
			Controller->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::GiveInitialItemsAuth);
		}
	}
}

static TCoroTask<void> GiveInitialItemsCoroutine(UInventoryComponent* Component, TArray<FInitialItemEntry> Items)
{
	for (const FInitialItemEntry& Entry : Items)
	{
		if (!Entry.IsValid())
		{
			UE_LOG(InventoryComponentLog, Warning, TEXT("Invalid initial item entry"));
			continue;
		}

		UItemInstance* Instance = co_await Component->AddItemAuthCoroutine(Entry.ItemDefinition, Entry.Count);

		if (!Instance)
		{
			UE_LOG(InventoryComponentLog, Warning, TEXT("초기 아이템 추가 실패: %s"), *Entry.ItemDefinition->DisplayName.ToString());
			continue;
		}

		if (Entry.bAddToQuickBar)
		{
			if (UQuickBarComponent* QuickBar = Component->GetOwner()->FindComponentByClass<UQuickBarComponent>())
			{
				QuickBar->AddItemToSlotAuth(Instance);
			}
		}
	}
}

void UInventoryComponent::GiveInitialItemsAuth(APawn* OldPawn, APawn* NewPawn)
{
	if (!NewPawn || bInitialItemsGiven)
	{
		return;
	}

	bInitialItemsGiven = true;

	if (InitialItems.Num() == 0)
	{
		return;
	}

	GiveInitialItemsCoroutine(this, InitialItems);
}

TCoroTask<UItemInstance*> UInventoryComponent::AddItemAuthCoroutine(const UItemDefinition* Definition, int32 Count)
{
	if (!Definition || !GetOwner() || !GetOwner()->HasAuthority())
	{
		co_return nullptr;
	}

	if (IsSlotFull())
	{
		UE_LOG(InventoryComponentLog, Warning, TEXT("인벤토리가 가득 찼습니다 (Max: %d)"), MaxSlots);
		co_return nullptr;
	}

	// 슬롯 예약
	int32 ReservedIndex = InventoryList.Entries.Num();
	InventoryList.Entries.AddDefaulted();

	// 번들 비동기 로딩
	co_await Coro::Async::LoadCommonDataAsset<UItemDefinition>(this, Definition);

	// 아이템 생성
	UItemInstance* Instance = AddItemAuthInternal(Definition, Count, ReservedIndex);
	co_return Instance;
}

UItemInstance* UInventoryComponent::AddItemAuthInternal(const UItemDefinition* Definition, int32 Count, int32 ReservedIndex)
{
	// Entry 설정 (리플리케이션 데이터)
	FInventoryEntry& Entry = InventoryList.Entries[ReservedIndex];
	Entry.Definition = Definition;
	Entry.ItemId = GenerateItemId();
	Entry.StackCount = Count;

	// Instance 생성
	UItemInstance* NewInstance = NewObject<UItemInstance>(this);
	NewInstance->Definition = Definition;
	NewInstance->ItemId = Entry.ItemId;
	NewInstance->ArrayIndex = ReservedIndex;

	// Fragment 초기화
	NewInstance->InitializeFragmentsFromDefinition();

	// Entry에 Instance 연결
	Entry.Instance = NewInstance;
	ItemMap.Add(NewInstance->ItemId, NewInstance);

	Entry.NotifyCreated(this);
	InventoryList.MarkEntryDirty(Entry);

	// 아이템 준비 완료 알림
	OnItemReady.Broadcast(NewInstance->ItemId, NewInstance);

	return NewInstance;
}

bool UInventoryComponent::RemoveItemAuth(UItemInstance* Instance)
{
	if (!Instance || !GetOwner() || !GetOwner()->HasAuthority())
	{
		return false;
	}

	const int32 Index = Instance->ArrayIndex;
	if (!InventoryList.Entries.IsValidIndex(Index))
	{
		return false;
	}

	const int32 LastIndex = InventoryList.Entries.Num() - 1;

	InventoryList.Entries[Index].NotifyRemoved(this);

	// 해당 아이템의 NetState 정리
	ItemNetStates.RemoveAllForItem(Instance->ItemId);

	ItemMap.Remove(Instance->ItemId);
	Instance->ArrayIndex = INDEX_NONE;

	// Swap된 아이템의 ArrayIndex 업데이트
	if (Index != LastIndex)
	{
		InventoryList.Entries[LastIndex].Instance->ArrayIndex = Index;
	}

	InventoryList.Entries.RemoveAtSwap(Index);
	InventoryList.MarkArrayDirty();

	return true;
}

bool UInventoryComponent::ModifyStackCountAuth(UItemInstance* Instance, int32 NewCount)
{
	if (!Instance || !GetOwner() || !GetOwner()->HasAuthority())
	{
		return false;
	}

	const int32 Index = Instance->ArrayIndex;
	if (!InventoryList.Entries.IsValidIndex(Index))
	{
		return false;
	}

	FInventoryEntry& Entry = InventoryList.Entries[Index];
	Entry.StackCount = NewCount;
	Entry.NotifyChanged(this);

	InventoryList.MarkEntryDirty(Entry);
	return true;
}

void UInventoryComponent::ModifyTagStatServer_Implementation(int32 ItemId, FGameplayTag StatTag, float NewValue)
{
	UItemInstance* Item = FindItemById(ItemId);
	if (!Item)
	{
		UE_LOG(InventoryComponentLog, Warning, TEXT("ModifyTagStatServer: ItemId=%d 아이템을 찾을 수 없습니다"), ItemId);
		return;
	}

	if (auto Modifier = Item->ModifyTagStatAuth(StatTag))
	{
		Modifier->Value = NewValue;
		UE_LOG(InventoryComponentLog, Log, TEXT("ModifyTagStatServer: ItemId=%d, %s = %.2f 설정 완료"), ItemId, *StatTag.ToString(), NewValue);
	}
	else
	{
		UE_LOG(InventoryComponentLog, Warning, TEXT("ModifyTagStatServer: ItemId=%d, %s TagStat을 찾을 수 없습니다"), ItemId, *StatTag.ToString());
	}
}

UItemInstance* UInventoryComponent::FindItemAtSlot(int32 SlotIndex) const
{
	if (InventoryList.Entries.IsValidIndex(SlotIndex))
	{
		return InventoryList.Entries[SlotIndex].Instance;
	}

	return nullptr;
}

UItemInstance* UInventoryComponent::FindItemById(int32 ItemId) const
{
	if (const TObjectPtr<UItemInstance>* Found = ItemMap.Find(ItemId))
	{
		return *Found;
	}

	return nullptr;
}

void UInventoryComponent::GetAllItems(TArray<UItemInstance*>& OutItems) const
{
	OutItems.Reset(InventoryList.Entries.Num());
	for (const FInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.Instance)
		{
			OutItems.Add(Entry.Instance);
		}
	}
}

FInventoryEntry* UInventoryComponent::FindEntry(const UItemInstance* Instance)
{
	if (!Instance)
	{
		return nullptr;
	}

	const int32 Index = Instance->ArrayIndex;
	if (InventoryList.Entries.IsValidIndex(Index))
	{
		return &InventoryList.Entries[Index];
	}

	return nullptr;
}

const FInventoryEntry* UInventoryComponent::FindEntry(const UItemInstance* Instance) const
{
	if (!Instance)
	{
		return nullptr;
	}

	const int32 Index = Instance->ArrayIndex;
	if (InventoryList.Entries.IsValidIndex(Index))
	{
		return &InventoryList.Entries[Index];
	}

	return nullptr;
}

TCoroTask<void> UInventoryComponent::InitializeReplicatedItemCoroutine(FInventoryEntry* Entry)
{
	if (!Entry || !Entry->Definition)
	{
		co_return;
	}

	// 에셋 로딩 대기
	co_await Coro::Async::LoadCommonDataAsset<UItemDefinition>(this, Entry->Definition);

	// 로컬 Instance 생성
	UItemInstance* NewInstance = NewObject<UItemInstance>(this);
	NewInstance->Definition = Entry->Definition;
	NewInstance->ItemId = Entry->ItemId;

	// Entry에 Instance 연결
	Entry->Instance = NewInstance;

	// ItemMap에 추가 (클라이언트에서도 ID로 조회 가능하도록)
	ItemMap.Add(NewInstance->ItemId, NewInstance);

	// Fragment 초기화
	NewInstance->InitializeFragmentsFromDefinition();

	// 기존 NetState 적용 (복제된 NetState가 있는 경우)
	for (const FItemNetStateEntry& NetStateEntry : ItemNetStates.Entries)
	{
		if (NetStateEntry.ItemId == NewInstance->ItemId)
		{
			FItemFragment* Fragment = NewInstance->FindFragmentByIndex(NetStateEntry.FragmentIndex);
			if (Fragment && NetStateEntry.NetState.IsValid())
			{
				NetStateEntry.NetState.Get()->ApplyToFragment(*Fragment);
			}
		}
	}

	NewInstance->OnAdded(this, Entry->StackCount);

	// 아이템 준비 완료 알림
	OnItemReady.Broadcast(NewInstance->ItemId, NewInstance);
}