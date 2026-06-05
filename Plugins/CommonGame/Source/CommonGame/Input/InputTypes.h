// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputTypes.generated.h"

class UInputAction;

/**
 * Enhanced Input Action과 GameplayTag를 매핑하는 구조체
 *
 * 입력 액션을 GameplayTag로 식별하여 데이터 기반 입력 시스템 구현에 사용됩니다
 */
USTRUCT(BlueprintType)
struct COMMONGAME_API FInputActionAndTag
{
	GENERATED_BODY()

public:
	bool IsValid() const
	{
		return InputTag.IsValid() && !InputAction.IsNull();
	}

	FString ToString() const;

public:
	/** 입력 액션 식별용 GameplayTag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FGameplayTag InputTag;

	/** Enhanced Input Action 에셋 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AssetBundles = "Client"))
	TSoftObjectPtr<UInputAction> InputAction;
};