// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CommonGameInstance.generated.h"

/**
 * CommonGame 기본 GameInstance
 *
 * 게임 시작 시 필요한 초기화 작업을 수행합니다.
 */
UCLASS()
class COMMONGAME_API UCommonGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	virtual void Init() override;
	virtual void Shutdown() override;
};