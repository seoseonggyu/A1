// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/CommonGameStateBase.h"
#include "A1GameState.generated.h"


/**
 * GameState
 *
 * �� �ý����� �����ϴ� TeamManagerComponent�� �����մϴ�.
 */
UCLASS()
class A1_API AA1GameState : public ACommonGameStateBase
{
	GENERATED_BODY()

public:
	AA1GameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

};
