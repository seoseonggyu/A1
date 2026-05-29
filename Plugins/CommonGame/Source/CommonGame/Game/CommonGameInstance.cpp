// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/CommonGameInstance.h"
// #include "Movement/MoveMode/MoveModeTypes.h" // TODO: 추가해야함
#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonGameInstance)

void UCommonGameInstance::Init()
{
	Super::Init();
}

void UCommonGameInstance::Shutdown()
{
	// FMoveModeRegistry::Get().Shutdown(); // TODO: 추가해야함

	Super::Shutdown();
}