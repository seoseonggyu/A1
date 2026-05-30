// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/CommonGameStateBase.h"
#include "A1GameState.generated.h"


/**
 * GameState
 *
 * 팀 시스템을 관리하는 TeamManagerComponent를 포함합니다.
 */
UCLASS()
class A1_API AA1GameState : public ACommonGameStateBase
{
	GENERATED_BODY()

public:
	AA1GameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//-----------------------------------------------------------------------------
	// 컴포넌트 접근자
	//-----------------------------------------------------------------------------

	// TODO: 팀 관련
	/** TeamManagerComponent를 반환합니다 */
	//UTeamManagerComponent* GetTeamManagerComponent() const { return TeamManagerComponent; }

//private:
//	/** 팀 관리 컴포넌트 */
//	UPROPERTY()
//	TObjectPtr<UTeamManagerComponent> TeamManagerComponent;
};
