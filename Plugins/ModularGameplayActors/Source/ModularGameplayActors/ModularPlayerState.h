// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ModularPlayerState.generated.h"

/**
 * 모듈식 게임플레이를 지원하는 PlayerState 베이스 클래스
 *
 * GameFrameworkComponentManager에 자동으로 등록되어
 * 모듈식 컴포넌트 추가 및 확장 이벤트를 수신할 수 있습니다.
 */
UCLASS()
class MODULARGAMEPLAYACTORS_API AModularPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AModularPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
