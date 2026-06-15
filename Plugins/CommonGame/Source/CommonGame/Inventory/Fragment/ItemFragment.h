// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ItemFragment.generated.h"

class UItemInstance;

DECLARE_LOG_CATEGORY_EXTERN(ItemFragmentLog, Log, All);

/**
 * 아이템의 개별 속성을 정의하는 Fragment 베이스 구조체
 *
 * 상속받아 다양한 Fragment를 구현합니다.
 * 예: FFragment_Stackable, FFragment_Equipable 등
 */
USTRUCT(BlueprintType, meta = (Hidden))
struct COMMONGAME_API FItemFragment
{
	GENERATED_BODY()

public:
	virtual ~FItemFragment() = default;

	/** Fragment가 생성될 때 호출됩니다 */
	virtual void OnCreated(UItemInstance* Owner) {}

	/** Fragment가 변경될 때 호출됩니다 */
	virtual void OnChanged(UItemInstance* Owner) {}
};