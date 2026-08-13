// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "Inventory/Fragment/ItemFragment.h"
#include "Inventory/Fragment/ItemFragment_TagStat.h"
#include "Inventory/Fragment/NetState/ItemFragmentNetStateModifier.h"
#include "ItemInstance.generated.h"

class UItemDefinition;
class UInventoryComponent;

DECLARE_LOG_CATEGORY_EXTERN(ItemInstanceLog, Log, All);

UCLASS(BlueprintType, Blueprintable)
class COMMONGAME_API UItemInstance : public UObject
{
	GENERATED_BODY()

	friend class UInventoryComponent;

public:
	virtual void OnAdded(UInventoryComponent* Inventory, int32 StackCount) {	}
	virtual void OnChanged(UInventoryComponent* Inventory, int32 StackCount){	}
	virtual void OnRemoved(UInventoryComponent* Inventory) {	}

	bool HasAuthority() const;
	void InitializeFragmentsFromDefinition();

	template<typename T>
	T* FindFragment()
	{
		for (TInstancedStruct<FItemFragment>& Entry : Fragments)
		{
			if (T* Found = Entry.GetMutablePtr<T>())
			{
				return Found;
			}
		}

		return nullptr;
	}

	template<typename T>
	const T* FindFragment() const
	{
		for (const TInstancedStruct<FItemFragment>& Entry : Fragments)
		{
			if (const T* Found = Entry.GetPtr<T>())
			{
				return Found;
			}
		}

		return nullptr;
	}
	
	template<typename T>
	bool HasFragment() const
	{
		return FindFragment<T>() != nullptr;
	}

	FItemFragment* FindFragmentByIndex(int32 Index);
	
	int32 GetFragmentIndex(const UScriptStruct* InFragmentType) const;

	void ResetFragmentToDefault(int32 FragmentIdx);
	
	template<typename TFragment>
	TItemFragmentNetStateModifier<typename TFragment::NetStateType> ModifyNetStateAuth()
	{
		if (!HasAuthority())
		{
			return {};
		}

		return ModifyNetStateByIndexAuth<typename TFragment::NetStateType, TFragment>(GetFragmentIndex(TFragment::StaticStruct()));
	}

	template<typename TFragment>
	bool RemoveNetStateAuth()
	{
		if (!HasAuthority())
		{
			return false;
		}

		return RemoveNetStateByIndex(GetFragmentIndex(TFragment::StaticStruct()));
	}

	TItemFragmentNetStateModifier<FNetState_TagStat> ModifyTagStatAuth(FGameplayTag InStatTag);


	bool RemoveTagStatAuth(FGameplayTag InStatTag);
	bool GetTagStatValue(FGameplayTag InStatTag, float& OutValue) const;
	bool SetTagStatValueLocal(FGameplayTag InStatTag, float NewValue);

	
	UFUNCTION(BlueprintCallable, Category = "Item|Equipment")
	FGameplayTag GetEquipmentSlotTag() const;

	UFUNCTION(BlueprintCallable, Category = "Item|Equipment")
	FGameplayTag GetQuickBarSlotTag() const;

private:

	int32 FindTagStatIndex(FGameplayTag InStatTag) const;
	
	template<typename TNetState, typename TFragment>
	TItemFragmentNetStateModifier<TNetState> ModifyNetStateByIndexAuth(int32 FragmentIdx);
	
	bool RemoveNetStateByIndex(int32 FragmentIdx);

	FItemNetStateList* GetOwnerNetStates() const;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	TObjectPtr<const UItemDefinition> Definition = nullptr;
	
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	int32 ItemId = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	int32 ArrayIndex = INDEX_NONE;

private:
	TArray<TInstancedStruct<FItemFragment>> Fragments;
};





template<typename TNetState, typename TFragment>
TItemFragmentNetStateModifier<TNetState> UItemInstance::ModifyNetStateByIndexAuth(int32 FragmentIdx)
{
	FItemNetStateList* NetStates = GetOwnerNetStates();
	if (FragmentIdx == INDEX_NONE || !NetStates)
	{
		return {};
	}

	TFragment* Fragment = Fragments[FragmentIdx].GetMutablePtr<TFragment>();
	
	int32 EntryIndex = NetStates->FindIndex(ItemId, static_cast<uint8>(FragmentIdx));
	TSharedPtr<TNetState> NetState;

	if (EntryIndex == INDEX_NONE)
	{
		NetState = MakeShared<TNetState>();

		FItemNetStateEntry& NewEntry = NetStates->Entries.AddDefaulted_GetRef();
		NewEntry.ItemId = ItemId;
		NewEntry.FragmentIndex = static_cast<uint8>(FragmentIdx);
		NewEntry.NetState = FItemFragmentNetStateHandle(NetState);
		EntryIndex = NetStates->Entries.Num() - 1;
	}
	else
	{
		NetState = StaticCastSharedPtr<TNetState>(NetStates->Entries[EntryIndex].NetState.Data);
	}

	return TItemFragmentNetStateModifier<TNetState>(NetState, Fragment, NetStates, EntryIndex, this);
}
