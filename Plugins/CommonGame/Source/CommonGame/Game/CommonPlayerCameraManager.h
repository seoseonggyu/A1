// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "CommonPlayerCameraManager.generated.h"

/**
 * CommonGame의 기본 PlayerCameraManager
 *
 */
UCLASS()
class COMMONGAME_API ACommonPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	ACommonPlayerCameraManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
