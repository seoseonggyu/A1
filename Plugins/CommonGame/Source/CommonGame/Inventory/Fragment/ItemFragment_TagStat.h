// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Inventory/Fragment/ItemFragment.h"
#include "Inventory/Fragment/NetState/ItemFragmentNetState.h"
#include "ItemFragment_TagStat.generated.h"

struct FFragment_TagStat;

//-----------------------------------------------------------------------------
// FNetState_TagStat
//-----------------------------------------------------------------------------

/**
 * TagStat Fragment의 네트워크 복제용 상태
 */
USTRUCT(BlueprintType)
struct COMMONGAME_API FNetState_TagStat : public FItemFragmentNetState
{
	GENERATED_BODY()

public:
	virtual UScriptStruct* GetScriptStruct() const override { return StaticStruct(); }
	virtual void ApplyToFragment(FItemFragment& Fragment) const override;

public:
	/** 현재 값 */
	UPROPERTY()
	float Value = 0.f;
};

//-----------------------------------------------------------------------------
// FFragment_TagStat
//-----------------------------------------------------------------------------

/**
 * GameplayTag로 식별되는 스탯 Fragment
 *
 * Definition에 여러 개 추가하여 다양한 스탯을 관리할 수 있습니다.
 * Tag는 에디터에서 설정하고, Value는 런타임에 네트워크로 변경 가능합니다.
 */
USTRUCT(BlueprintType, DisplayName = "Tag Stat")
struct COMMONGAME_API FFragment_TagStat : public FItemFragment
{
	GENERATED_BODY()

public:
	/** NetState 타입 정의 (ModifyNetStateAuth에서 사용) */
	using NetStateType = FNetState_TagStat;

public:
	/** 스탯을 식별하는 태그 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TagStat", meta = (Categories = "Stat"))
	FGameplayTag StatTag;

	/** 현재 값 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TagStat")
	float Value = 0.f;
};
