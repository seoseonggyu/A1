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

/**
 * ��Ÿ�� ������ �ν��Ͻ�
 *
 * Fragment ������� ������ �Ӽ��� �������� �����մϴ�.
 * Fragment�� Definition���� �ʱ�ȭ�ǰ�, NetState�� ��Ʈ��ũ �����˴ϴ�.
 */
UCLASS(BlueprintType, Blueprintable)
class COMMONGAME_API UItemInstance : public UObject
{
	GENERATED_BODY()

	friend class UInventoryComponent;

public:
	//-----------------------------------------------------------------------------
	// Inventory �ݹ�
	//-----------------------------------------------------------------------------

	/** �κ��丮�� �߰��� �� ȣ��˴ϴ� */
	virtual void OnAdded(UInventoryComponent* Inventory, int32 StackCount) {}

	/** �κ��丮 ������ ����� �� ȣ��˴ϴ� */
	virtual void OnChanged(UInventoryComponent* Inventory, int32 StackCount) {}

	/** �κ��丮���� ���ŵ� �� ȣ��˴ϴ� */
	virtual void OnRemoved(UInventoryComponent* Inventory) {}

	/** ���� ������ �ִ��� Ȯ���մϴ� */
	bool HasAuthority() const;

	//-----------------------------------------------------------------------------
	// �ʱ�ȭ
	//-----------------------------------------------------------------------------

	/** Definition���� Fragment���� �ʱ�ȭ�մϴ� (����/Ŭ���̾�Ʈ ���) */
	void InitializeFragmentsFromDefinition();

	//-----------------------------------------------------------------------------
	// Fragment API (��ȸ)
	//-----------------------------------------------------------------------------

	/** Fragment�� �˻��մϴ� */
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

	/** Fragment�� �˻��մϴ� (const) */
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

	/** Ư�� Ÿ���� Fragment�� �ִ��� Ȯ���մϴ� */
	template<typename T>
	bool HasFragment() const
	{
		return FindFragment<T>() != nullptr;
	}


	/** �ε����� Fragment�� �˻��մϴ� */
	FItemFragment* FindFragmentByIndex(int32 Index);

	/** Fragment Ÿ���� �ε����� ��ȯ�մϴ� */
	int32 GetFragmentIndex(const UScriptStruct* InFragmentType) const;

	/** Fragment�� Definition�� �⺻������ �����մϴ� */
	void ResetFragmentToDefault(int32 FragmentIdx);

	//-----------------------------------------------------------------------------
	// NetState API (���� ���� - ��Ʈ��ũ ����)
	//-----------------------------------------------------------------------------

	/**
	 * Fragment�� NetState�� �����մϴ� (���� ����)
	 *
	 * NetState�� ������ �����ϰ�, ������ ������Ʈ�մϴ�.
	 * ������ ���� �� �ڵ����� Fragment�� ���� + MarkDirty �˴ϴ�.
	 * @code
	 * if (auto Modifier = ItemInstance->ModifyNetStateAuth<FFragment_Stackable>())
	 * {
	 *     Modifier->CurrentStack = 50;
	 * } // ������ ���� �� �ڵ����� Fragment ���� + ��Ʈ��ũ ����
	 * @endcode
	 */
	template<typename TFragment>
	TItemFragmentNetStateModifier<typename TFragment::NetStateType> ModifyNetStateAuth()
	{
		if (!HasAuthority())
		{
			return {};
		}

		return ModifyNetStateByIndexAuth<typename TFragment::NetStateType, TFragment>(GetFragmentIndex(TFragment::StaticStruct()));
	}

	/**
	 * Fragment�� NetState�� �����մϴ� (���� ����)
	 *
	 * NetState�� �����ϸ� Ŭ���̾�Ʈ���� Fragment�� Definition�� �⺻������ �����˴ϴ�.
	 *
	 * @tparam TFragment Fragment Ÿ��
	 * @return NetState�� �����Ͽ� ���ŵǾ����� true
	 */
	template<typename TFragment>
	bool RemoveNetStateAuth()
	{
		if (!HasAuthority())
		{
			return false;
		}

		return RemoveNetStateByIndex(GetFragmentIndex(TFragment::StaticStruct()));
	}

	/**
	 * Ư�� �±��� TagStat NetState�� �����մϴ� (���� ����)
	 *
	 * @param InStatTag ã�� ���� �±�
	 * @return �ش� �±��� TagStat Modifier (������ ��ȿ)
	 * @code
	 * if (auto Modifier = ItemInstance->ModifyTagStatAuth(StatTag_Damage))
	 * {
	 *     Modifier->Value = 75.f;
	 * }
	 * @endcode
	 */
	TItemFragmentNetStateModifier<FNetState_TagStat> ModifyTagStatAuth(FGameplayTag InStatTag);

	/**
	 * Ư�� �±��� TagStat NetState�� �����մϴ� (���� ����)
	 *
	 * NetState�� �����ϸ� Ŭ���̾�Ʈ���� Definition�� �⺻������ �����˴ϴ�.
	 *
	 * @param InStatTag ã�� ���� �±�
	 * @return NetState�� �����Ͽ� ���ŵǾ����� true
	 */
	bool RemoveTagStatAuth(FGameplayTag InStatTag);

	//-----------------------------------------------------------------------------
	// TagStat API (��ȸ)
	//-----------------------------------------------------------------------------

	/**
	 * Ư�� �±��� TagStat ���� ��ȯ�մϴ�
	 *
	 * @param InStatTag ã�� ���� �±�
	 * @param OutValue ã�� �� (���)
	 * @return �ش� �±׸� ���� TagStat�� ������ true
	 */
	bool GetTagStatValue(FGameplayTag InStatTag, float& OutValue) const;

	/**
	 * Ư�� �±��� TagStat ���� ���ÿ��� ���� �����մϴ� (��Ʈ��ũ ���� ����)
	 *
	 * Ŭ���̾�Ʈ/������ ���� ���ÿ��� ����ϴ� ���� ����մϴ�.
	 * ��: ź�� (�����Ƽ ���� ����)
	 *
	 * @param InStatTag ã�� ���� �±�
	 * @param NewValue �� ��
	 * @return �ش� �±׸� ���� TagStat�� ������ true
	 */
	bool SetTagStatValueLocal(FGameplayTag InStatTag, float NewValue);



	//-----------------------------------------------------------------------------
	// Equipment ����
	//-----------------------------------------------------------------------------

	/** Equipment ���� �±׸� ��ȯ�մϴ� */
	UFUNCTION(BlueprintCallable, Category = "Item|Equipment")
	FGameplayTag GetEquipmentSlotTag() const;

	/** QuickBar ���� �±׸� ��ȯ�մϴ� */
	UFUNCTION(BlueprintCallable, Category = "Item|Equipment")
	FGameplayTag GetQuickBarSlotTag() const;

private:
	//-----------------------------------------------------------------------------
	// NetState ���� ����
	//-----------------------------------------------------------------------------


	/** Ư�� StatTag�� ���� TagStat Fragment�� �ε����� ã���ϴ� */
	int32 FindTagStatIndex(FGameplayTag InStatTag) const;

	/** Fragment �ε����� NetState�� �����մϴ� (ã�ų� ����) */
	template<typename TNetState, typename TFragment>
	TItemFragmentNetStateModifier<TNetState> ModifyNetStateByIndexAuth(int32 FragmentIdx);

	/** Fragment �ε����� NetState�� �����մϴ� */
	bool RemoveNetStateByIndex(int32 FragmentIdx);

	/** InventoryComponent�� ItemNetStates�� ��ȯ�մϴ� (���� ����) */
	FItemNetStateList* GetOwnerNetStates() const;

public:
	/** ������ ���� (CDO) */
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	TObjectPtr<const UItemDefinition> Definition = nullptr;

	/** ���� ������ ID */
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	int32 ItemId = INDEX_NONE;

	/** �κ��丮 �迭 �� �ε��� */
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	int32 ArrayIndex = INDEX_NONE;

private:
	/** Fragment ����Ʈ (����/Ŭ���̾�Ʈ�� ������ Definition���� ����) */
	TArray<TInstancedStruct<FItemFragment>> Fragments;
};

//-----------------------------------------------------------------------------
// ���ø� ����
//-----------------------------------------------------------------------------

template<typename TNetState, typename TFragment>
TItemFragmentNetStateModifier<TNetState> UItemInstance::ModifyNetStateByIndexAuth(int32 FragmentIdx)
{
	FItemNetStateList* NetStates = GetOwnerNetStates();
	if (FragmentIdx == INDEX_NONE || !NetStates)
	{
		return {};
	}

	TFragment* Fragment = Fragments[FragmentIdx].GetMutablePtr<TFragment>();

	// NetState ã�ų� ���� (ItemId + FragmentIndex�� �˻�)
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
