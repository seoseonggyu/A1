// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "CommonPlayerCameraManager.generated.h"

#define COMMON_CAMERA_DEFAULT_FOV		(80.0f)
#define COMMON_CAMERA_DEFAULT_PITCH_MIN	(-89.0f)
#define COMMON_CAMERA_DEFAULT_PITCH_MAX	(89.0f)

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
