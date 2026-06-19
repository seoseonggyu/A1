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

	// 팀 배정 (ChoosePlayerStart보다 먼저 호출되어야 함)
	if (AA1PlayerController* A1PlayerController = Cast<AA1PlayerController>(PlayerController))
	{
		// TODO: 팀 관련
		/*if (UTeamManagerComponent* TeamManager = GetTeamManagerComponent())
		{
			TeamManager->AutoAssignTeamAuth(CSPlayerController);
		}*/
	}

	return PlayerController;
}

void AA1GameMode::Logout(AController* Exiting)
{
	// TODO: 팀관련
	// 팀에서 제거
	/*if (ACSPlayerController* CSPlayerController = Cast<ACSPlayerController>(Exiting))
	{
		if (UTeamManagerComponent* TeamManager = GetTeamManagerComponent())
		{
			TeamManager->RemovePlayerFromTeamAuth(CSPlayerController);
		}
	}*/

	Super::Logout(Exiting);
}

AActor* AA1GameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	// TODO: 팀관련 소환?
	return Super::ChoosePlayerStart_Implementation(Player);
}
