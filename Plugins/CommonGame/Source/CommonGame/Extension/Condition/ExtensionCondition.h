// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExtensionCondition.generated.h"

class AActor;

/**
 * ActorExtension 활성화 조건을 정의하는 베이스 구조체
 *
 * 서브클래스에서 IsSatisfied()를 구현하여 조건 평가 로직을 정의합니다.
 * Extension에 여러 Condition을 추가하면 AND 조합으로 평가됩니다.
 */
USTRUCT(BlueprintType, meta=(Hidden))
struct COMMONGAME_API FExtensionCondition
{
	GENERATED_BODY()

public:
	virtual ~FExtensionCondition() = default;

	/** 조건 충족 여부를 확인합니다 */
	virtual bool IsSatisfied(AActor* Owner) const { return true; }
};
