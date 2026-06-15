// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "Inventory/Fragment/ItemFragment.h"
// #include "Inventory/Fragment/ItemFragment_TagStat.h" // TODO: TagStat 추가
#include "Inventory/Fragment/NetState/ItemFragmentNetStateModifier.h"
#include "ItemInstance.generated.h"

class UItemDefinition;
class UInventoryComponent;

DECLARE_LOG_CATEGORY_EXTERN(ItemInstanceLog, Log, All);

/**
 * 런타임 아이템 인스턴스
 *
 * Fragment 기반으로 아이템 속성을 동적으로 구성합니다.
 * Fragment는 Definition에서 초기화되고, NetState만 네트워크 복제됩니다.
 */
UCLASS(BlueprintType, Blueprintable)
class COMMONGAME_API UItemInstance : public UObject
{
	GENERATED_BODY()

	friend class UInventoryComponent;

public:
	//-----------------------------------------------------------------------------
	// Inventory 콜백
	//-----------------------------------------------------------------------------

	/** 인벤토리에 추가될 때 호출됩니다 */
	virtual void OnAdded(UInventoryComponent* Inventory, int32 StackCount) {}

	/** 인벤토리 내에서 변경될 때 호출됩니다 */
	virtual void OnChanged(UInventoryComponent* Inventory, int32 StackCount) {}

	/** 인벤토리에서 제거될 때 호출됩니다 */
	virtual void OnRemoved(UInventoryComponent* Inventory) {}

	/** 서버 권한이 있는지 확인합니다 */
	bool HasAuthority() const;

	//-----------------------------------------------------------------------------
	// 초기화
	//-----------------------------------------------------------------------------

	/** Definition에서 Fragment들을 초기화합니다 (서버/클라이언트 모두) */
	void InitializeFragmentsFromDefinition();

	//-----------------------------------------------------------------------------
	// Fragment API (조회)
	//-----------------------------------------------------------------------------

	/** Fragment를 검색합니다 */
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

	/** Fragment를 검색합니다 (const) */
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

	/** 특정 타입의 Fragment가 있는지 확인합니다 */
	template<typename T>
	bool HasFragment() const
	{
		return FindFragment<T>() != nullptr;
	}


	/** 인덱스로 Fragment를 검색합니다 */
	FItemFragment* FindFragmentByIndex(int32 Index);

	/** Fragment 타입의 인덱스를 반환합니다 */
	int32 GetFragmentIndex(const UScriptStruct* InFragmentType) const;

	/** Fragment를 Definition의 기본값으로 복원합니다 */
	void ResetFragmentToDefault(int32 FragmentIdx);

	//-----------------------------------------------------------------------------
	// NetState API (서버 전용 - 네트워크 복제)
	//-----------------------------------------------------------------------------

	/**
	 * Fragment의 NetState를 수정합니다 (서버 전용)
	 *
	 * NetState가 없으면 생성하고, 있으면 업데이트합니다.
	 * 스코프 종료 시 자동으로 Fragment에 적용 + MarkDirty 됩니다.
	 * @code
	 * if (auto Modifier = ItemInstance->ModifyNetStateAuth<FFragment_Stackable>())
	 * {
	 *     Modifier->CurrentStack = 50;
	 * } // 스코프 종료 시 자동으로 Fragment 적용 + 네트워크 복제
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
	 * Fragment의 NetState를 제거합니다 (서버 전용)
	 *
	 * NetState를 제거하면 클라이언트에서 Fragment가 Definition의 기본값으로 복원됩니다.
	 *
	 * @tparam TFragment Fragment 타입
	 * @return NetState가 존재하여 제거되었으면 true
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

	// TODO: 여기서 TagStat NetState 수정



	//-----------------------------------------------------------------------------
	// Equipment 헬퍼
	//-----------------------------------------------------------------------------

	/** Equipment 슬롯 태그를 반환합니다 */
	UFUNCTION(BlueprintCallable, Category = "Item|Equipment")
	FGameplayTag GetEquipmentSlotTag() const;

	/** QuickBar 슬롯 태그를 반환합니다 */
	UFUNCTION(BlueprintCallable, Category = "Item|Equipment")
	FGameplayTag GetQuickBarSlotTag() const;

private:
	//-----------------------------------------------------------------------------
	// NetState 내부 헬퍼
	//-----------------------------------------------------------------------------


	/** 특정 StatTag를 가진 TagStat Fragment의 인덱스를 찾습니다 */
	// int32 FindTagStatIndex(FGameplayTag InStatTag) const; TODO: 여기서 TagStat NetState 수정

	/** Fragment 인덱스로 NetState를 수정합니다 (찾거나 생성) */
	template<typename TNetState, typename TFragment>
	TItemFragmentNetStateModifier<TNetState> ModifyNetStateByIndexAuth(int32 FragmentIdx);

	/** Fragment 인덱스로 NetState를 제거합니다 */
	bool RemoveNetStateByIndex(int32 FragmentIdx);

	/** InventoryComponent의 ItemNetStates를 반환합니다 (서버 전용) */
	FItemNetStateList* GetOwnerNetStates() const;

public:
	/** 아이템 정의 (CDO) */
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	TObjectPtr<const UItemDefinition> Definition = nullptr;

	/** 고유 아이템 ID */
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	int32 ItemId = INDEX_NONE;

	/** 인벤토리 배열 내 인덱스 */
	UPROPERTY(BlueprintReadOnly, Category = "Item")
	int32 ArrayIndex = INDEX_NONE;

private:
	/** Fragment 리스트 (서버/클라이언트가 동일한 Definition에서 복사) */
	TArray<TInstancedStruct<FItemFragment>> Fragments;
};

//-----------------------------------------------------------------------------
// 템플릿 구현
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

	// NetState 찾거나 생성 (ItemId + FragmentIndex로 검색)
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
