// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Extension/Condition/ExtensionCondition.h"
#include "Templates/SubclassOf.h"
#include "ExtensionCondition_HasInputComponent.generated.h"

class UInputComponent;

/**
 * Pawn에 특정 InputComponent가 있는지 확인하는 Condition
 *
 * RequiredClass가 지정되면 해당 타입인지도 확인합니다.
 * Pawn이 아닌 Actor에 적용하면 항상 false를 반환합니다.
 */
USTRUCT(BlueprintType, meta = (DisplayName = "Has Input Component"))
struct COMMONGAME_API FExtensionCondition_HasInputComponent : public FExtensionCondition
{
	GENERATED_BODY()

public:
	virtual bool IsSatisfied(AActor* Owner) const override;

public:
	/** 요구하는 InputComponent 클래스 (nullptr이면 기본 UInputComponent만 확인) */
	UPROPERTY(EditAnywhere, Category = "Condition", meta = (AllowAbstract = "false"))
	TSubclassOf<UInputComponent> RequiredClass;
};
