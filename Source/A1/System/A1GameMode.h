// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/CommonGameModeBase.h"
#include "A1GameMode.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(A1GameModeLog, Log, All);

/**
 * 프로젝트의 GameMode 클래스
 *
 * CommonGameModeBase를 상속받아 Experience 시스템을 지원합니다.
 * 플레이어 접속/퇴장 시 팀 배정을 관리합니다.
 */
UCLASS()
class A1_API AA1GameMode : public ACommonGameModeBase
{
	GENERATED_BODY()

public:
	AA1GameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//-----------------------------------------------------------------------------
	// AGameModeBase 오버라이드
	//-----------------------------------------------------------------------------

	virtual APlayerController* SpawnPlayerControllerCommon(ENetRole InRemoteRole, FVector const& SpawnLocation, FRotator const& SpawnRotation, TSubclassOf<APlayerController> InPlayerControllerClass) override;
	virtual void Logout(AController* Exiting) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	// TOOD: 팀 관련
//private:
//	/** TeamManagerComponent를 반환합니다 */
//	UTeamManagerComponent* GetTeamManagerComponent() const;
};