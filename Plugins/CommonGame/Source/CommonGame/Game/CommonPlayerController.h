// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularPlayerController.h"
#include "CommonPlayerController.generated.h"

/** PostProcessInput 완료 후 호출되는 델리게이트 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPostProcessInput, float /*DeltaTime*/, bool /*bGamePaused*/);

/**
 * CommonGame의 기본 PlayerController
 *
 * 서버에서 Possess 시 AbilitySystem을 초기화합니다.
 * PostProcessInput 시점에 OnPostProcessInput 델리게이트를 브로드캐스트합니다.
 * UCommonLocalPlayer의 델리게이트를 Broadcast합니다 (PlayerController/PlayerState/Pawn 설정 시).
 */

UCLASS()
class COMMONGAME_API ACommonPlayerController : public AModularPlayerController
{
	GENERATED_BODY()

public:
	ACommonPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ReceivedPlayer() override;
	virtual void SetPawn(APawn* InPawn) override;
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;

protected:
	virtual void OnRep_PlayerState() override;


	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;


public:
	/** PostProcessInput 시 호출되는 델리게이트 */
	FOnPostProcessInput OnPostProcessInput;

};