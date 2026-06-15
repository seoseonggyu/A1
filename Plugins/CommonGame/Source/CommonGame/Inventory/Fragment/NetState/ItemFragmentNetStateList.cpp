// Copyright Epic Games, Inc. All Rights Reserved.

#include "ItemFragmentNetStateList.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemInstance.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(ItemFragmentNetStateList)

void FItemNetStateEntry::PostReplicatedAdd(const FItemNetStateList& InArraySerializer)
{
	if (!InArraySerializer.Owner)
	{
		return;
	}

	UItemInstance* Item = InArraySerializer.Owner->FindItemById(ItemId);
	if (!Item)
	{
		return;
	}

	if (!NetState.IsValid())
	{
		return;
	}

	if (FItemFragment* Fragment = Item->FindFragmentByIndex(FragmentIndex))
	{
		NetState.Get()->ApplyToFragment(*Fragment);
	}
}

void FItemNetStateEntry::PostReplicatedChange(const FItemNetStateList& InArraySerializer)
{
	// Add와 동일한 처리
	PostReplicatedAdd(InArraySerializer);
}

void FItemNetStateEntry::PreReplicatedRemove(const FItemNetStateList& InArraySerializer)
{
	if (!InArraySerializer.Owner)
	{
		return;
	}

	// Fragment를 Definition의 기본값으로 복원
	if (UItemInstance* Item = InArraySerializer.Owner->FindItemById(ItemId))
	{
		Item->ResetFragmentToDefault(FragmentIndex);
	}
}
