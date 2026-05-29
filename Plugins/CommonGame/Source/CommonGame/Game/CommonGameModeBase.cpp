// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/CommonGameModeBase.h"
#include "Game/CommonCharacter.h"
#include "Game/CommonGameStateBase.h"
#include "Game/CommonHUD.h"
#include "Game/CommonPlayerController.h"
#include "Game/CommonPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonGameModeBase)

DEFINE_LOG_CATEGORY(CommonGameModeLog);

ACommonGameModeBase::ACommonGameModeBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GameStateClass = ACommonGameStateBase::StaticClass();
	PlayerControllerClass = ACommonPlayerController::StaticClass();
	PlayerStateClass = ACommonPlayerState::StaticClass();
	DefaultPawnClass = ACommonCharacter::StaticClass();
	HUDClass = ACommonHUD::StaticClass();
}

void ACommonGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	StartExperienceLoadCoroutine();
}

TCoroTask<void> ACommonGameModeBase::StartExperienceLoadCoroutine()
{

	return TCoroTask<void>();
}


void ACommonGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
}

UClass* ACommonGameModeBase::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	return nullptr;
}

APawn* ACommonGameModeBase::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	return nullptr;
}

bool ACommonGameModeBase::IsExperienceLoaded() const
{
	return false;
}

void ACommonGameModeBase::OnExperienceLoaded(const UExperienceDefinition* Experience)
{
}

