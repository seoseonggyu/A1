// Copyright Epic Games, Inc. All Rights Reserved.

#include "Inventory/InventoryComponent.h"

#include "IPropertyTable.h"
#include "Inventory/ItemDefinition.h"
#include "Inventory/Fragment/ItemFragment_Equipment.h"
#include "Equipment/EquipmentComponent.h"
#include "Equipment/EquipmentInstance.h"
#include "Equipment/EquipmentDefinition.h"
#include "Coroutine/CommonAssetAwaiters.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
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

	// TODO: Client에서 자체적으로 제거?
	// Instance = nullptr;
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

	// UI(클라이언트)가 위치/스택 변경을 그리드에 반영하도록 알림. Added는 InitializeReplicatedItemCoroutine에서 별도 처리
	if (Owner)
	{
		Owner->OnInventoryGridChanged.Broadcast(EInventoryGridChangeType::Changed, ItemId, Instance, SlotPosition, StackCount);
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
		
		if (Entry.bAddToEquipment)
		{
			Component->ToEquipmentFromInventoryAuth(Instance);
		}
	}
}


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
	DOREPLIFETIME_WITH_PARAMS_FAST(UInventoryComponent, OccupiedCells, Params);
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// 서버와 클라 모두 사용
	OccupiedCells.Init(false, GridSize.X * GridSize.Y);

	// 서버에서만 Pawn Possess 시 초기 아이템 지급
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (AController* Controller = Cast<AController>(GetOwner()))
		{
			Controller->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::GiveInitialItemsAuth);
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

	const FIntPoint Size = GetSizeFromDefinition(Definition);
	if (Size == FIntPoint::ZeroValue)
	{
		co_return nullptr;
	}

	FIntPoint SlotPos;
	if (!FindEmptySlot(Size, SlotPos))
	{
		UE_LOG(InventoryComponentLog, Warning, TEXT("AddItemAuthCoroutine: 그리드에 빈 공간이 없습니다 (%dx%d)"), Size.X, Size.Y);
		co_return nullptr;
	}

	// 찾은 영역을 즉시 점유 상태로 표시 (비동기 로딩 대기 중 다른 추가와의 경쟁 방지)
	SetGridOccupiedAuth(SlotPos, Size, true);

	// 슬롯 예약
	int32 ReservedIndex = InventoryList.Entries.Num();
	InventoryList.Entries.AddDefaulted();

	// 번들 비동기 로딩
	co_await Coro::Async::LoadCommonDataAsset<UItemDefinition>(this, Definition);

	// 아이템 인스턴스 생성
	UItemInstance* Instance = AddItemAuthInternal(Definition, Count, SlotPos, ReservedIndex);
	co_return Instance;
}

UItemInstance* UInventoryComponent::AddItemAuthInternal(const UItemDefinition* Definition, int32 Count, FIntPoint SlotPos, int32 ReservedIndex)
{
	// Entry 설정 (리플리케이션 데이터)
	FInventoryEntry& Entry = InventoryList.Entries[ReservedIndex];
	Entry.Definition = Definition;
	Entry.ItemId = GenerateItemId();
	Entry.StackCount = Count;
	Entry.SlotPosition = SlotPos;
	Entry.bEquipment = false;

	// Instance 생성
	UItemInstance* NewInstance = NewObject<UItemInstance>(this);
	NewInstance->Definition = Entry.Definition;
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

	const FIntPoint Size = GetSizeFromDefinition(InventoryList.Entries[Index].Definition);
	SetGridOccupiedAuth(InventoryList.Entries[Index].SlotPosition, Size, false);

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

void UInventoryComponent::ToEquipmentFromInventoryAuth(UItemInstance* Instance)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !Instance)
	{
		return;
	}
	
	const int32 Index = Instance->ArrayIndex;
	if (!InventoryList.Entries.IsValidIndex(Index))
	{
		return;
	}
	
	const AController* Controller = Cast<AController>(GetOwner());
	UEquipmentComponent* EquipmentComponent = UEquipmentComponent::FindEquipmentComponent(Controller->GetPawn());
	if (!EquipmentComponent)
	{
		UE_LOG(InventoryComponentLog, Warning, TEXT("초기 아이템 장착 실패: EquipmentComponent를 찾을 수 없습니다"));
		return;
	}

	// 아이템 장착
	EquipmentComponent->HandleActiveEquipChangedAuth(Instance);

	const FIntPoint Size = GetSizeFromDefinition(InventoryList.Entries[Index].Definition);
	SetGridOccupiedAuth(InventoryList.Entries[Index].SlotPosition, Size, false);

	InventoryList.Entries[Index].SlotPosition = FIntPoint(-1, -1);
	InventoryList.Entries[Index].bEquipment = true;
	InventoryList.Entries[Index].NotifyChanged(this);
	InventoryList.MarkEntryDirty(InventoryList.Entries[Index]);
}

void UInventoryComponent::EquipFromInventoryServer_Implementation(int32 ItemId, FGameplayTag SlotTag)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	UItemInstance* NewItem = FindItemById(ItemId);
	if (!NewItem)
	{
		UE_LOG(InventoryComponentLog, Warning, TEXT("EquipFromInventoryServer: ItemId=%d 아이템을 찾을 수 없습니다"), ItemId);
		return;
	}

	// 이 아이템이 실제로 SlotTag에 장착 가능한지 서버에서 재검증합니다 (클라이언트의 CanAcceptItem은 UX용일 뿐 신뢰하지 않음).
	const FItemFragment_Equipment* Fragment = NewItem->FindFragment<FItemFragment_Equipment>();
	if (!Fragment || !Fragment->EquipmentDefinition || Fragment->EquipmentDefinition->SlotTag != SlotTag)
	{
		UE_LOG(InventoryComponentLog, Warning, TEXT("EquipFromInventoryServer: ItemId=%d는 슬롯 %s에 장착할 수 없습니다"), ItemId, *SlotTag.ToString());
		return;
	}

	FInventoryEntry* NewEntry = FindEntry(NewItem);
	if (!NewEntry)
	{
		return;
	}

	const AController* Controller = Cast<AController>(GetOwner());
	UEquipmentComponent* EquipmentComponent = Controller ? UEquipmentComponent::FindEquipmentComponent(Controller->GetPawn()) : nullptr;
	if (!EquipmentComponent)
	{
		UE_LOG(InventoryComponentLog, Warning, TEXT("EquipFromInventoryServer: EquipmentComponent를 찾을 수 없습니다"));
		return;
	}

	// 대상 슬롯에 이미 장착된 아이템이 있으면, 인벤토리로 되돌릴 대상 Entry를 미리 확보해둡니다.
	// (실제 해제/데이터 파괴는 아래 HandleActiveEquipChangedAuth가 수행하므로, 그 전에 원본 아이템을 알아둬야 합니다)
	UItemInstance* OldItem = nullptr;
	FInventoryEntry* OldEntry = nullptr;
	if (UEquipmentInstance* OldEquip = EquipmentComponent->GetEquipmentInSlot(SlotTag))
	{
		OldItem = OldEquip->GetSourceItemInstance();
		OldEntry = OldItem ? FindEntry(OldItem) : nullptr;
	}

	// 새 아이템을 그리드에서 먼저 내립니다. 기존 장비를 되돌릴 빈 칸을 찾을 때
	// 이 칸도 후보에 포함되어야 하므로(같은 자리로 교체되는 경우), 검색보다 먼저 비웁니다.
	const FIntPoint NewSize = GetSizeFromDefinition(NewEntry->Definition);
	const FIntPoint NewOldSlotPos = NewEntry->SlotPosition;
	SetGridOccupiedAuth(NewOldSlotPos, NewSize, false);

	// 기존 장착 아이템이 있다면 되돌릴 빈 칸을 찾습니다. 없으면 전체 작업을 취소합니다(부분 적용 방지).
	FIntPoint OldItemAnchor(-1, -1);
	if (OldEntry)
	{
		const FIntPoint OldSize = GetSizeFromDefinition(OldEntry->Definition);
		if (!FindEmptySlot(OldSize, OldItemAnchor))
		{
			// 실패: 방금 비운 새 아이템의 그리드 점유를 원복하고 중단합니다.
			SetGridOccupiedAuth(NewOldSlotPos, NewSize, true);
			UE_LOG(InventoryComponentLog, Warning, TEXT("EquipFromInventoryServer: 기존 장비를 되돌릴 빈 칸이 없어 교체를 취소합니다"));
			return;
		}
	}

	// 새 아이템 Entry를 장착 상태로 전환합니다.
	NewEntry->SlotPosition = FIntPoint(-1, -1);
	NewEntry->bEquipment = true;
	NewEntry->NotifyChanged(this);
	InventoryList.MarkEntryDirty(*NewEntry);

	// 장착 처리 (내부적으로 대상 슬롯에 남아있는 기존 장비 데이터를 해제합니다)
	EquipmentComponent->HandleActiveEquipChangedAuth(NewItem);

	// 기존 장착 아이템을 인벤토리 그리드로 되돌립니다.
	if (OldEntry)
	{
		const FIntPoint OldSize = GetSizeFromDefinition(OldEntry->Definition);
		SetGridOccupiedAuth(OldItemAnchor, OldSize, true);

		OldEntry->SlotPosition = OldItemAnchor;
		OldEntry->bEquipment = false;
		OldEntry->NotifyChanged(this);
		InventoryList.MarkEntryDirty(*OldEntry);
	}
}

void UInventoryComponent::UnequipToInventoryServer_Implementation(int32 ItemId, FGameplayTag FromEquipmentSlotTag, FIntPoint Anchor)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	UItemInstance* Item = FindItemById(ItemId);
	if (!Item)
	{
		UE_LOG(InventoryComponentLog, Warning, TEXT("UnequipToInventoryServer: ItemId=%d 아이템을 찾을 수 없습니다"), ItemId);
		return;
	}

	FInventoryEntry* Entry = FindEntry(Item);
	if (!Entry || !Entry->bEquipment)
	{
		return;
	}

	const FIntPoint Size = GetSizeFromDefinition(Entry->Definition);
	if (!CanPlaceAt(Anchor, Size))
	{
		UE_LOG(InventoryComponentLog, Warning, TEXT("UnequipToInventoryServer: 대상 위치(%d, %d)에 배치할 수 없습니다"), Anchor.X, Anchor.Y);
		return;
	}

	const AController* Controller = Cast<AController>(GetOwner());
	UEquipmentComponent* EquipmentComponent = Controller ? UEquipmentComponent::FindEquipmentComponent(Controller->GetPawn()) : nullptr;
	if (!EquipmentComponent)
	{
		UE_LOG(InventoryComponentLog, Warning, TEXT("UnequipToInventoryServer: EquipmentComponent를 찾을 수 없습니다"));
		return;
	}

	// 클라이언트가 보낸 슬롯에 실제로 이 아이템이 장착되어 있는지 서버에서 재검증합니다.
	UEquipmentInstance* Equipped = EquipmentComponent->GetEquipmentInSlot(FromEquipmentSlotTag);
	if (!Equipped || Equipped->GetSourceItemInstance() != Item)
	{
		UE_LOG(InventoryComponentLog, Warning, TEXT("UnequipToInventoryServer: ItemId=%d가 슬롯 %s에 장착되어 있지 않습니다"), ItemId, *FromEquipmentSlotTag.ToString());
		return;
	}

	EquipmentComponent->UnequipItemAuth(FromEquipmentSlotTag);

	SetGridOccupiedAuth(Anchor, Size, true);
	Entry->SlotPosition = Anchor;
	Entry->bEquipment = false;
	Entry->NotifyChanged(this);
	InventoryList.MarkEntryDirty(*Entry);
}

void UInventoryComponent::SetOccupiedCellsAuth(int32 CellPos, bool Value)
{
	if (!GetOwner()->HasAuthority() || CellPos < 0 || CellPos >= OccupiedCells.Num())
	{
		return;
	}

	OccupiedCells[CellPos] = Value;
	MARK_PROPERTY_DIRTY_FROM_NAME(UInventoryComponent, OccupiedCells, this);
}

void UInventoryComponent::SetGridOccupiedAuth(const FIntPoint& Anchor, const FIntPoint& Size, bool bValue)
{
	if (Anchor.X < 0 || Anchor.Y < 0)
	{
		return;
	}

	for (int32 OffsetY = 0; OffsetY < Size.Y; ++OffsetY)
	{
		for (int32 OffsetX = 0; OffsetX < Size.X; ++OffsetX)
		{
			const int32 CellIndex = (Anchor.Y + OffsetY) * GridSize.X + (Anchor.X + OffsetX);
			SetOccupiedCellsAuth(CellIndex, bValue);
		}
	}
}

FInventoryEntry* UInventoryComponent::FindEntry(const UItemInstance* Instance)
{
	if (!Instance)
	{
		return nullptr;
	}

	// 서버 fast-path: ArrayIndex로 O(1) 조회 (검증까지 통과해야 신뢰).
	const int32 Index = Instance->ArrayIndex;
	if (InventoryList.Entries.IsValidIndex(Index) && InventoryList.Entries[Index].Instance == Instance)
	{
		return &InventoryList.Entries[Index];
	}

	// 클라이언트는 ArrayIndex를 유지하지 않는다(=INDEX_NONE)
	// 폴백: ItemId로 선형 탐색 (클라이언트 및 fast-path 실패 시). 인벤토리 크기가 작아 부담 없음.
	for (FInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.ItemId == Instance->ItemId)
		{
			return &Entry;
		}
	}

	return nullptr;
}

const FInventoryEntry* UInventoryComponent::FindEntry(const UItemInstance* Instance) const
{
	if (!Instance)
	{
		return nullptr;
	}

	// 서버 fast-path: ArrayIndex로 O(1) 조회 (검증까지 통과해야 신뢰).
	// 클라이언트는 ArrayIndex를 유지하지 않으므로(=INDEX_NONE) 아래 폴백으로 넘어간다.
	const int32 Index = Instance->ArrayIndex;
	if (InventoryList.Entries.IsValidIndex(Index) && InventoryList.Entries[Index].Instance == Instance)
	{
		return &InventoryList.Entries[Index];
	}

	// 폴백: ItemId로 선형 탐색 (클라이언트 및 fast-path 실패 시). 인벤토리 크기가 작아 부담 없음.
	for (const FInventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.ItemId == Instance->ItemId)
		{
			return &Entry;
		}
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

	// 그리드 배치를 UI에 알림 (클라이언트)
	OnInventoryGridChanged.Broadcast(EInventoryGridChangeType::Added, Entry->ItemId, NewInstance, Entry->SlotPosition, Entry->StackCount);
}

//-----------------------------------------------------------------------------
// 2D 그리드
//-----------------------------------------------------------------------------


FIntPoint UInventoryComponent::GetSizeFromDefinition(const UItemDefinition* InDefinition)
{
	return InDefinition ? InDefinition->SlotCount : FIntPoint::ZeroValue;
}

FIntPoint UInventoryComponent::GetSlotPosition(const UItemInstance* Instance) const
{
	if (const FInventoryEntry* Entry = FindEntry(Instance))
	{
		return Entry->SlotPosition;
	}

	return FIntPoint(-1, -1);
}


//-----------------------------------------------------------------------------
// Cell 관리
//-----------------------------------------------------------------------------

bool UInventoryComponent::FindEmptySlot(const FIntPoint& Size, FIntPoint& OutSlotPos) const
{
	if (Size.X <= 0 || Size.Y <= 0 || Size.X > GridSize.X || Size.Y > GridSize.Y)
	{
		return false;
	}

	for (int32 AnchorY = 0; AnchorY <= GridSize.Y - Size.Y; ++AnchorY)
	{
		for (int32 AnchorX = 0; AnchorX <= GridSize.X - Size.X; ++AnchorX)
		{
			if (CanPlaceAt(FIntPoint(AnchorX, AnchorY), Size))
			{
				OutSlotPos = FIntPoint(AnchorX, AnchorY);
				return true;
			}
		}
	}

	return false;
}

bool UInventoryComponent::CanPlaceAt(const FIntPoint& Anchor, const FIntPoint& Size, const UItemInstance* IgnoreInstance) const
{
	if (Size.X <= 0 || Size.Y <= 0
		|| Anchor.X < 0 || Anchor.Y < 0
		|| Anchor.X + Size.X > GridSize.X || Anchor.Y + Size.Y > GridSize.Y)
	{
		return false;
	}

	// IgnoreInstance가 이미 차지한 칸은 점유로 치지 않는다 (같은 아이템을 겹치는 위치로 옮기는 경우의 오탐 방지)
	FIntPoint IgnoreAnchor(-1, -1);
	FIntPoint IgnoreSize = FIntPoint::ZeroValue;
	if (const FInventoryEntry* IgnoreEntry = FindEntry(IgnoreInstance))
	{
		IgnoreAnchor = IgnoreEntry->SlotPosition;
		IgnoreSize = GetSizeFromDefinition(IgnoreEntry->Definition);
	}

	for (int32 OffsetY = 0; OffsetY < Size.Y; ++OffsetY)
	{
		for (int32 OffsetX = 0; OffsetX < Size.X; ++OffsetX)
		{
			const int32 X = Anchor.X + OffsetX;
			const int32 Y = Anchor.Y + OffsetY;

			if (IgnoreAnchor.X >= 0 &&
				X >= IgnoreAnchor.X && X < IgnoreAnchor.X + IgnoreSize.X &&
				Y >= IgnoreAnchor.Y && Y < IgnoreAnchor.Y + IgnoreSize.Y)
			{
				continue;
			}

			const int32 CellIndex = Y * GridSize.X + X;
			if (OccupiedCells[CellIndex])
			{
				return false;
			}
		}
	}

	return true;
}

bool UInventoryComponent::MoveItemAuth(UItemInstance* Instance, const FIntPoint& NewAnchor)
{
	if (!Instance || !GetOwner() || !GetOwner()->HasAuthority())
	{
		return false;
	}

	FInventoryEntry* Entry = FindEntry(Instance);
	if (!Entry)
	{
		return false;
	}

	const FIntPoint Size = GetSizeFromDefinition(Entry->Definition);
	if (!CanPlaceAt(NewAnchor, Size, Instance))
	{
		return false;
	}

	SetGridOccupiedAuth(Entry->SlotPosition, Size, false);
	SetGridOccupiedAuth(NewAnchor, Size, true);

	Entry->SlotPosition = NewAnchor;
	Entry->NotifyChanged(this);
	InventoryList.MarkEntryDirty(*Entry);

	return true;
}

void UInventoryComponent::MoveItemServer_Implementation(int32 ItemId, FIntPoint NewAnchor)
{
	UItemInstance* Instance = FindItemById(ItemId);
	if (!Instance)
	{
		UE_LOG(InventoryComponentLog, Warning, TEXT("MoveItemServer: ItemId=%d 아이템을 찾을 수 없습니다"), ItemId);
		return;
	}

	MoveItemAuth(Instance, NewAnchor);
}
