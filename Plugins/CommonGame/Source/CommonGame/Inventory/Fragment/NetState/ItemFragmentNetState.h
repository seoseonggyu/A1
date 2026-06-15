// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ItemFragmentNetState.generated.h"

struct FItemFragment;

/**
 * Fragment의 네트워크 복제용 상태 베이스 구조체
 *
 * TPolymorphicStructNetSerializerImpl을 사용하여 효율적인 네트워크 직렬화를 지원합니다.
 * 파생 클래스는 반드시 GetScriptStruct()를 오버라이드해야 합니다.
 */
USTRUCT(BlueprintType, meta = (Hidden))
struct COMMONGAME_API FItemFragmentNetState
{
	GENERATED_BODY()

public:
	virtual ~FItemFragmentNetState() = default;

	/**
	 * 실제 UScriptStruct 타입을 반환합니다
	 *
	 * TPolymorphicStructNetSerializerImpl이 타입을 식별하는 데 사용합니다.
	 * 파생 클래스에서 반드시 오버라이드해야 합니다.
	 */
	virtual UScriptStruct* GetScriptStruct() const PURE_VIRTUAL(FItemFragmentNetState::GetScriptStruct, return nullptr;);

	/** NetState 값을 Fragment에 적용합니다 */
	virtual void ApplyToFragment(FItemFragment& Fragment) const PURE_VIRTUAL(FItemFragmentNetState::ApplyToFragment, );
};
