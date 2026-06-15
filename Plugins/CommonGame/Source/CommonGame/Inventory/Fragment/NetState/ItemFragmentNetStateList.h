// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Iris/ReplicationState/IrisFastArraySerializer.h"
#include "ItemFragmentNetStateHandle.h"
#include "ItemFragmentNetStateList.generated.h"

class UInventoryComponent;
class UItemInstance;

//-----------------------------------------------------------------------------
// FItemNetStateEntry
//-----------------------------------------------------------------------------

/**
 * 아이템 Fragment의 네트워크 상태를 담는 항목
 *
 * UInventoryComponent 레벨의 최상위 FastArraySerializer에서 관리됩니다.
 * 중첩 FastArraySerializer 문제를 피하기 위해 FInventoryEntry 밖에 위치합니다.
 */
USTRUCT(BlueprintType)
struct COMMONGAME_API FItemNetStateEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	friend class UInventoryComponent;
	friend class UItemInstance;

public:
	void PostReplicatedAdd(const struct FItemNetStateList& InArraySerializer);
	void PostReplicatedChange(const struct FItemNetStateList& InArraySerializer);
	void PreReplicatedRemove(const struct FItemNetStateList& InArraySerializer);

public:
	/** 아이템 ID */
	UPROPERTY()
	int32 ItemId = INDEX_NONE;

	/** Fragment 배열 인덱스 (최대 16개) */
	UPROPERTY()
	uint8 FragmentIndex = 0;

	/** NetState 데이터 */
	UPROPERTY()
	FItemFragmentNetStateHandle NetState;
};

//-----------------------------------------------------------------------------
// FItemNetStateList
//-----------------------------------------------------------------------------

/**
 * 아이템 NetState를 담는 최상위 FastArraySerializer 컨테이너
 *
 * UInventoryComponent에서 직접 소유하여 중첩 FastArraySerializer 문제를 피합니다.
 * 각 Entry는 ItemId로 어떤 아이템인지, FragmentIndex로 어떤 Fragment인지 식별합니다.
 */
USTRUCT(BlueprintType)
struct COMMONGAME_API FItemNetStateList : public FIrisFastArraySerializer
{
	GENERATED_BODY()

public:
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FItemNetStateEntry, FItemNetStateList>(Entries, DeltaParms, *this);
	}

	/** 특정 ItemId와 FragmentIndex에 해당하는 Entry 인덱스를 찾습니다 */
	int32 FindIndex(int32 ItemId, uint8 FragmentIndex) const
	{
		for (int32 i = 0; i < Entries.Num(); ++i)
		{
			if (Entries[i].ItemId == ItemId && Entries[i].FragmentIndex == FragmentIndex)
			{
				return i;
			}
		}
		return INDEX_NONE;
	}

	/** 특정 ItemId의 모든 Entry를 제거합니다 */
	void RemoveAllForItem(int32 ItemId)
	{
		for (int32 i = Entries.Num() - 1; i >= 0; --i)
		{
			if (Entries[i].ItemId == ItemId)
			{
				Entries.RemoveAtSwap(i);
			}
		}
		MarkArrayDirty();
	}

	/** 특정 인덱스의 Entry를 Dirty로 마킹합니다 */
	void MarkEntryDirty(int32 Index)
	{
		if (Entries.IsValidIndex(Index))
		{
			MarkItemDirty(Entries[Index]);
		}
	}

public:
	/** NetState 배열 */
	UPROPERTY()
	TArray<FItemNetStateEntry> Entries;

	/** 소유 InventoryComponent (복제되지 않음) */
	UPROPERTY(NotReplicated)
	TObjectPtr<UInventoryComponent> Owner = nullptr;
};

template<>
struct TStructOpsTypeTraits<FItemNetStateList> : public TStructOpsTypeTraitsBase2<FItemNetStateList>
{
	enum { WithNetDeltaSerializer = true };
};
