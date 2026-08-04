// Copyright Epic Games, Inc. All Rights Reserved.

#include "Equipment/QuickBarComponent.h"

#include "EquipmentInstance.h"
#include "Equipment/EquipmentComponent.h"
#include "Inventory/ItemInstance.h"
#include "Inventory/InventoryComponent.h"
#include "Awaiters/Delegate.h"
#include "Net/UnrealNetwork.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(QuickBarComponent)

DEFINE_LOG_CATEGORY(QuickBarComponentLog);

//-----------------------------------------------------------------------------
// FQuickBarEntry
//-----------------------------------------------------------------------------

void FQuickBarEntry::PostReplicatedAdd(const FQuickBarList& InArraySerializer)
{
	if (ItemId == INDEX_NONE)
	{
		return;
	}

	if (InArraySerializer.Owner)
	{
		InArraySerializer.Owner->WaitForItemInitializedCoroutine(ItemId, SlotIndex);
	}
}

void FQuickBarEntry::PostReplicatedChange(const FQuickBarList& InArraySerializer)
{
}

void FQuickBarEntry::PreReplicatedRemove(const FQuickBarList& InArraySerializer)
{
	if (ItemId == INDEX_NONE || !InArraySerializer.Owner)
	{
		return;
	}

	// InventoryComponent에서 Item을 찾아서 SlotMap에서 제거합니다
	if (UInventoryComponent* Inventory = InArraySerializer.Owner->GetOwner()->FindComponentByClass<UInventoryComponent>())
	{
		if (UItemInstance* Item = Inventory->FindItemById(ItemId))
		{
			InArraySerializer.Owner->RemoveFromSlotMap(Item, Item->GetQuickBarSlotTag(), SlotIndex);
		}
	}
}

//-----------------------------------------------------------------------------
// UQuickBarComponent
//-----------------------------------------------------------------------------

UQuickBarComponent::UQuickBarComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	bWantsInitializeComponent = true;
	QuickBarList.Owner = this;
}

void UQuickBarComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// 슬롯별 Items 배열을 MaxCount 크기로 미리 할당합니다
	for (auto& [Tag, SlotData] : QuickBarSlots)
	{
		SlotData.Items.SetNum(SlotData.MaxCount);
	}
}

void UQuickBarComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	Params.Condition = COND_OwnerOnly;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, QuickBarList, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ActiveSlot, Params);
}

bool UQuickBarComponent::AddItemToSlotAuth(UItemInstance* Item)
{
	if (!Item)
	{
		return false;
	}

	const FGameplayTag SlotTag = Item->GetQuickBarSlotTag();

	// 슬롯 태그가 유효한지 확인합니다
	if (!SlotTag.IsValid())
	{
		UE_LOG(QuickBarComponentLog, Warning, TEXT("AddItemToSlotAuth: 아이템에 QuickBarSlotTag가 없습니다"));
		return false;
	}

	// 슬롯이 등록되어 있는지 확인합니다
	if (!QuickBarSlots.Contains(SlotTag))
	{
		UE_LOG(QuickBarComponentLog, Warning, TEXT("AddItemToSlotAuth: 등록되지 않은 슬롯입니다 (%s)"), *SlotTag.ToString());
		return false;
	}

	FQuickBarSlotData& SlotData = QuickBarSlots[SlotTag];

	// 슬롯이 가득 찼는지 확인합니다
	if (SlotData.IsFull())
	{
		UE_LOG(QuickBarComponentLog, Warning, TEXT("AddItemToSlotAuth: 슬롯이 가득 찼습니다 (%s, Max: %d)"), *SlotTag.ToString(), SlotData.MaxCount);
		return false;
	}

	// 이미 등록된 아이템인지 확인합니다
	if (IsItemRegistered(Item))
	{
		UE_LOG(QuickBarComponentLog, Warning, TEXT("AddItemToSlotAuth: 이미 등록된 아이템입니다"));
		return false;
	}

	// 빈 슬롯 인덱스를 찾습니다
	const int32 NewSlotIndex = SlotData.FindEmptyIndex();
	if (NewSlotIndex == INDEX_NONE)
	{
		UE_LOG(QuickBarComponentLog, Warning, TEXT("AddItemToSlotAuth: 빈 슬롯을 찾을 수 없습니다 (%s)"), *SlotTag.ToString());
		return false;
	}

	// Entry에 추가합니다
	FQuickBarEntry& NewEntry = QuickBarList.Entries.AddDefaulted_GetRef();
	NewEntry.ItemId = Item->ItemId;
	NewEntry.SlotIndex = NewSlotIndex;
	QuickBarList.MarkEntryDirty(NewEntry);

	// 슬롯 맵에 추가합니다
	AddToSlotMap(Item, SlotTag, NewSlotIndex);

	// QuickBar에 등록된 아이템은 인벤토리 그리드에서 제외합니다 (장착된 것으로 취급)
	// if (UInventoryComponent* Inventory = UInventoryComponent::FindInventoryComponent(Cast<AController>(GetOwner())))
	// {
	// 	Inventory->UnplaceItemAuth(Item->ItemId);
	// }

	// 현재 장착된 아이템이 없으면 자동 장착합니다
	if (!ActiveSlot.IsValid())
	{
		SetActiveSlotAuth(SlotTag, NewSlotIndex);
	}

	return true;
}

bool UQuickBarComponent::RemoveItemFromSlotAuth(UItemInstance* Item)
{
	if (!Item)
	{
		return false;
	}

	const int32 EntryIndex = FindEntryIndex(Item);
	if (EntryIndex == INDEX_NONE)
	{
		UE_LOG(QuickBarComponentLog, Warning, TEXT("RemoveItemFromSlotAuth: 등록되지 않은 아이템입니다"));
		return false;
	}

	const FQuickBarEntry& Entry = QuickBarList.Entries[EntryIndex];
	const FGameplayTag SlotTag = Item->GetQuickBarSlotTag();

	// 활성 슬롯의 아이템이면 장비 해제합니다
	if (SlotTag == ActiveSlot.SlotTag && Entry.SlotIndex == ActiveSlot.Index)
	{
		UnequipItem(Item);
	}

	// 슬롯 맵에서 제거합니다
	RemoveFromSlotMap(Item, SlotTag, Entry.SlotIndex);

	// Entry에서 제거합니다
	QuickBarList.Entries.RemoveAtSwap(EntryIndex);
	QuickBarList.MarkArrayDirty();

	// QuickBar에서 빠진 아이템은 다시 인벤토리 그리드의 빈 칸에 배치합니다
	// if (UInventoryComponent* Inventory = UInventoryComponent::FindInventoryComponent(Cast<AController>(GetOwner())))
	// {
	// 	Inventory->PlaceAtEmptySlotAuth(Item->ItemId);
	// }

	return true;
}

void UQuickBarComponent::SetActiveSlotAuth(FGameplayTag SlotTag, int32 Index)
{
	// 슬롯이 등록되어 있는지 확인합니다
	if (!QuickBarSlots.Contains(SlotTag))
	{
		UE_LOG(QuickBarComponentLog, Warning, TEXT("SetActiveSlotAuth: 등록되지 않은 슬롯입니다 (%s)"), *SlotTag.ToString());
		return;
	}

	const FQuickBarSlotData& SlotData = QuickBarSlots[SlotTag];

	// 인덱스가 유효한지 확인합니다
	if (Index < 0 || Index >= SlotData.Num())
	{
		UE_LOG(QuickBarComponentLog, Warning, TEXT("SetActiveSlotAuth: 유효하지 않은 인덱스입니다 (%s, Index: %d, Count: %d)"), *SlotTag.ToString(), Index, SlotData.Num());
		return;
	}

	FQuickBarActiveSlot NewSlot;
	NewSlot.SlotTag = SlotTag;
	NewSlot.Index = Index;

	if (ActiveSlot != NewSlot)
	{
		const FQuickBarActiveSlot OldSlot = ActiveSlot;
		ActiveSlot = NewSlot;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ActiveSlot, this);

		HandleActiveSlotChanged(OldSlot, NewSlot);
	}
}

bool UQuickBarComponent::CanSetActiveSlot(FGameplayTag SlotTag, int32 Index) const
{
	// 슬롯이 등록되어 있는지
	if (!IsSlotRegistered(SlotTag))
	{
		UE_LOG(QuickBarComponentLog, Warning, TEXT("CanSetActiveSlot: 등록되지 않은 슬롯입니다 (%s)"), *SlotTag.ToString());
		return false;
	}

	// 해당 인덱스에 아이템이 있는지
	const FQuickBarSlotData* SlotData = GetSlotData(SlotTag);
	if (!SlotData || !SlotData->Items.IsValidIndex(Index) || !SlotData->Items[Index])
	{
		UE_LOG(QuickBarComponentLog, Warning, TEXT("CanSetActiveSlot: 슬롯에 아이템이 없습니다 (%s[%d])"), *SlotTag.ToString(), Index);
		return false;
	}

	// 이미 활성 슬롯인지
	if (ActiveSlot.SlotTag == SlotTag && ActiveSlot.Index == Index)
	{
		return false;
	}

	return true;
}

void UQuickBarComponent::SetActiveSlotServer_Implementation(FGameplayTag SlotTag, int32 Index)
{
	if (!CanSetActiveSlot(SlotTag, Index))
	{
		return;
	}

	SetActiveSlotAuth(SlotTag, Index);
}

UItemInstance* UQuickBarComponent::GetActiveItem() const
{
	if (!ActiveSlot.IsValid())
	{
		return nullptr;
	}

	const FQuickBarSlotData* SlotData = GetSlotData(ActiveSlot.SlotTag);
	if (!SlotData || !SlotData->Items.IsValidIndex(ActiveSlot.Index))
	{
		return nullptr;
	}

	return SlotData->Items[ActiveSlot.Index];
}

const FQuickBarSlotData* UQuickBarComponent::GetSlotData(FGameplayTag SlotTag) const
{
	return QuickBarSlots.Find(SlotTag);
}

bool UQuickBarComponent::IsSlotFull(FGameplayTag SlotTag) const
{
	const FQuickBarSlotData* SlotData = GetSlotData(SlotTag);
	return SlotData ? SlotData->IsFull() : true;
}

bool UQuickBarComponent::IsItemRegistered(UItemInstance* Item) const
{
	if (!Item)
	{
		return false;
	}

	const FGameplayTag SlotTag = Item->GetQuickBarSlotTag();
	const FQuickBarSlotData* SlotData = QuickBarSlots.Find(SlotTag);
	return SlotData && SlotData->Items.Contains(Item);
}

void UQuickBarComponent::AddToSlotMap(UItemInstance* Item, FGameplayTag SlotTag, int32 SlotIndex)
{
	if (!Item || !SlotTag.IsValid())
	{
		return;
	}

	if (FQuickBarSlotData* SlotData = QuickBarSlots.Find(SlotTag))
	{
		if (SlotData->Items.IsValidIndex(SlotIndex))
		{
			SlotData->Items[SlotIndex] = Item;
		}
	}
}

void UQuickBarComponent::RemoveFromSlotMap(UItemInstance* Item, FGameplayTag SlotTag, int32 SlotIndex)
{
	if (!Item || !SlotTag.IsValid())
	{
		return;
	}

	if (FQuickBarSlotData* SlotData = QuickBarSlots.Find(SlotTag))
	{
		if (SlotData->Items.IsValidIndex(SlotIndex) && SlotData->Items[SlotIndex] == Item)
		{
			SlotData->Items[SlotIndex] = nullptr;
		}
	}
}

int32 UQuickBarComponent::FindEntryIndex(UItemInstance* Item) const
{
	if (!Item)
	{
		return INDEX_NONE;
	}

	for (int32 i = 0; i < QuickBarList.Entries.Num(); ++i)
	{
		if (QuickBarList.Entries[i].ItemId == Item->ItemId)
		{
			return i;
		}
	}

	return INDEX_NONE;
}

void UQuickBarComponent::HandleActiveSlotChanged(const FQuickBarActiveSlot& OldSlot, const FQuickBarActiveSlot& NewSlot)
{
	// 장비 장착/해제는 서버에서만 처리합니다
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	// 기존 아이템 해제
	if (OldSlot.IsValid())
	{
		if (const FQuickBarSlotData* OldSlotData = GetSlotData(OldSlot.SlotTag))
		{
			if (OldSlotData->Items.IsValidIndex(OldSlot.Index))
			{
				UnequipItem(OldSlotData->Items[OldSlot.Index]);
			}
		}
	}
	
	if (NewSlot.IsValid())
	{
		if (const FQuickBarSlotData* NewSlotData = GetSlotData(NewSlot.SlotTag))
		{
			if (NewSlotData->Items.IsValidIndex(NewSlot.Index))
			{
				EquipItemCoroutine(NewSlotData->Items[NewSlot.Index]);
			}
		}
	}
}

TCoroTask<void> UQuickBarComponent::EquipItemCoroutine(UItemInstance* Item)
{
	if (!Item)
	{
		co_return;
	}

	UEquipmentComponent* EquipmentComponent = GetEquipmentComponent();
	if (!EquipmentComponent)
	{
		co_return;
	}

	EquippedItem = co_await EquipmentComponent->EquipItemAuthCoroutine(Item);
	co_return;
}

void UQuickBarComponent::UnequipItem(const UItemInstance* Item)
{
	if (!Item || !EquippedItem)
	{
		return;
	}

	UEquipmentComponent* EquipmentComponent = GetEquipmentComponent();
	if (!EquipmentComponent)
	{
		return;
	}

	EquipmentComponent->UnequipItemAuth(EquippedItem);
	EquippedItem = nullptr;
}

UEquipmentComponent* UQuickBarComponent::GetEquipmentComponent() const
{
	if (CachedEquipmentComponent)
	{
		return CachedEquipmentComponent;
	}

	AController* Controller = Cast<AController>(GetOwner());
	if (!Controller)
	{
		return nullptr;
	}

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn)
	{
		return nullptr;
	}

	CachedEquipmentComponent = Pawn->FindComponentByClass<UEquipmentComponent>();
	return CachedEquipmentComponent;
}

TCoroTask<void> UQuickBarComponent::WaitForItemInitializedCoroutine(int32 ItemId, int32 SlotIndex)
{
	if (ItemId == INDEX_NONE)
	{
		co_return;
	}

	// InventoryComponent에서 아이템 조회
	UInventoryComponent* Inventory = GetOwner()->FindComponentByClass<UInventoryComponent>();
	if (!Inventory)
	{
		co_return;
	}

	// 아이템이 이미 준비되었는지 확인
	UItemInstance* Item = Inventory->FindItemById(ItemId);

	// 아이템이 없으면 OnItemReady 대기
	while (!Item)
	{
		auto [ReadyItemId, ReadyItem] = co_await Coro::Async::WaitForDelegate(this, Inventory->OnItemReady);

		if (ReadyItemId == ItemId)
		{
			Item = ReadyItem;
		}
	}

	const FGameplayTag SlotTag = Item->GetQuickBarSlotTag();
	AddToSlotMap(Item, SlotTag, SlotIndex);
}
