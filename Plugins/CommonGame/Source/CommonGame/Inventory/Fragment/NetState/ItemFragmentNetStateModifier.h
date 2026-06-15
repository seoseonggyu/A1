// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Fragment/ItemFragment.h"
#include "Inventory/Fragment/NetState/ItemFragmentNetState.h"
#include "Inventory/Fragment/NetState/ItemFragmentNetStateHandle.h"
#include "Inventory/Fragment/NetState/ItemFragmentNetStateList.h"

class UItemInstance;

/**
 * Fragment NetState 수정용 RAII 래퍼
 *
 * 스코프 종료 시 자동으로:
 * 1. NetState 값을 Fragment에 적용
 * 2. MarkDirty 호출 (네트워크 복제 트리거)
 * 3. Fragment의 OnChanged 호출
 */
template<typename TNetState>
struct TItemFragmentNetStateModifier
{
public:
	TItemFragmentNetStateModifier() = default;

	TItemFragmentNetStateModifier(TSharedPtr<TNetState> InNetState, FItemFragment* InFragment, FItemNetStateList* InNetStateList, int32 InIndex, UItemInstance* InOwner)
		: NetState(InNetState), Fragment(InFragment), NetStateList(InNetStateList), Index(InIndex), Owner(InOwner) {}

	~TItemFragmentNetStateModifier()
	{
		if (NetState && Fragment && NetStateList && Owner)
		{
			// NetState 값을 Fragment에 적용
			NetState->ApplyToFragment(*Fragment);

			// 네트워크 복제 마킹
			NetStateList->MarkEntryDirty(Index);

			// Fragment 변경 알림
			Fragment->OnChanged(Owner);
		}
	}

	// 복사 금지
	TItemFragmentNetStateModifier(const TItemFragmentNetStateModifier&) = delete;
	TItemFragmentNetStateModifier& operator=(const TItemFragmentNetStateModifier&) = delete;

	// 이동 허용
	TItemFragmentNetStateModifier(TItemFragmentNetStateModifier&& Other) noexcept
		: NetState(MoveTemp(Other.NetState)), Fragment(Other.Fragment), NetStateList(Other.NetStateList), Index(Other.Index), Owner(Other.Owner)
	{
		Other.Fragment = nullptr;
		Other.NetStateList = nullptr;
		Other.Index = INDEX_NONE;
		Other.Owner = nullptr;
	}

	TItemFragmentNetStateModifier& operator=(TItemFragmentNetStateModifier&& Other) noexcept
	{
		if (this != &Other)
		{
			NetState = MoveTemp(Other.NetState);
			Fragment = Other.Fragment;
			NetStateList = Other.NetStateList;
			Index = Other.Index;
			Owner = Other.Owner;

			Other.Fragment = nullptr;
			Other.NetStateList = nullptr;
			Other.Index = INDEX_NONE;
			Other.Owner = nullptr;
		}
		return *this;
	}

public:
	TNetState* operator->() const { return NetState.Get(); }
	TNetState& operator*() const { return *NetState; }
	explicit operator bool() const { return NetState.IsValid(); }

private:
	TSharedPtr<TNetState> NetState;
	FItemFragment* Fragment = nullptr;
	FItemNetStateList* NetStateList = nullptr;
	int32 Index = INDEX_NONE;
	UItemInstance* Owner = nullptr;
};
