// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/CommonPlayerCameraManager.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonPlayerCameraManager)

ACommonPlayerCameraManager::ACommonPlayerCameraManager(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	DefaultFOV = COMMON_CAMERA_DEFAULT_FOV;
	ViewPitchMin = COMMON_CAMERA_DEFAULT_PITCH_MIN;
	ViewPitchMax = COMMON_CAMERA_DEFAULT_PITCH_MAX;
}
