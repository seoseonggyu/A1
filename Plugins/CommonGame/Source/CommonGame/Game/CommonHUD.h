// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularHUD.h"
#include "CommonHUD.generated.h"

/**
 * CommonGame의 기본 HUD
 *
 * ModularHUD를 상속받아 모듈식 컴포넌트 시스템을 지원합니다.
 */
UCLASS()
class COMMONGAME_API ACommonHUD : public AModularHUD
{
	GENERATED_BODY()

public:
	ACommonHUD(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};