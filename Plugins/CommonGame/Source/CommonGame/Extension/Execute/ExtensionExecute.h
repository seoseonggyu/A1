// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExtensionExecute.generated.h"

class AActor;

/**
 * ActorExtension 활성화 시 실행되는 작업을 정의하는 베이스 구조체
 *
 * 서브클래스에서 OnActivate/OnDeactivate를 구현하여 실행 로직을 정의합니다.
 * mutable 키워드로 런타임 상태를 관리할 수 있습니다.
 */
USTRUCT(BlueprintType, meta=(Hidden))
struct COMMONGAME_API FExtensionExecute
{
	GENERATED_BODY()

public:
	virtual ~FExtensionExecute() = default;

	/** 활성화 시 실행됩니다 */
	virtual void OnActivate(AActor* Owner) const {}

	/** 비활성화 시 실행됩니다 */
	virtual void OnDeactivate(AActor* Owner) const {}
};
