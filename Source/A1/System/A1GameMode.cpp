#include "A1GameMode.h"

#include "Player/A1PlayerController.h"
#include "Player/A1Character.h"
#include "Player/A1PlayerState.h"
#include "System/A1GameState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1GameMode)

DEFINE_LOG_CATEGORY(A1GameModeLog);

AA1GameMode::AA1GameMode(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PlayerControllerClass = AA1PlayerController::StaticClass();
    DefaultPawnClass = AA1Character::StaticClass();
	PlayerStateClass = AA1PlayerState::StaticClass();
    GameStateClass = AA1GameState::StaticClass();
}

APlayerController* AA1GameMode::SpawnPlayerControllerCommon(ENetRole InRemoteRole, FVector const& SpawnLocation, FRotator const& SpawnRotation, TSubclassOf<APlayerController> InPlayerControllerClass)
{
	APlayerController* PlayerController = Super::SpawnPlayerControllerCommon(InRemoteRole, SpawnLocation, SpawnRotation, InPlayerControllerClass);


	return PlayerController;
}

void AA1GameMode::Logout(AController* Exiting)
{

	Super::Logout(Exiting);
}

AActor* AA1GameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	return Super::ChoosePlayerStart_Implementation(Player);
}
