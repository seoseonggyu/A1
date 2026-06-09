// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/LocalPlayer.h"
#include "CommonLocalPlayer.generated.h"

class APlayerController;
class APlayerState;
class APawn;

/**
 * 확장된 LocalPlayer 클래스
 *
 * PlayerController, PlayerState, Pawn 설정 시점을 델리게이트로 알려줍니다.
 * UI 시스템에서 PlayerController가 설정된 후 레이아웃을 생성하는 데 사용됩니다.
 */
UCLASS(MinimalAPI, Config = Engine, Transient)
class UCommonLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()

public:
	UCommonLocalPlayer();

	//-----------------------------------------------------------------------------
	// 델리게이트 선언
	//-----------------------------------------------------------------------------

	/** PlayerController가 설정되었을 때 호출됩니다 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerControllerSetDelegate, UCommonLocalPlayer* /*LocalPlayer*/, APlayerController* /*PlayerController*/);
	FPlayerControllerSetDelegate OnPlayerControllerSet;

	/** PlayerState가 설정되었을 때 호출됩니다 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerStateSetDelegate, UCommonLocalPlayer* /*LocalPlayer*/, APlayerState* /*PlayerState*/);
	FPlayerStateSetDelegate OnPlayerStateSet;

	/** Pawn이 설정되었을 때 호출됩니다 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerPawnSetDelegate, UCommonLocalPlayer* /*LocalPlayer*/, APawn* /*Pawn*/);
	FPlayerPawnSetDelegate OnPlayerPawnSet;

	//-----------------------------------------------------------------------------
	// 델리게이트 헬퍼 함수
	//-----------------------------------------------------------------------------

	/**
	 * 델리게이트를 등록하고, PlayerController가 이미 존재하면 즉시 호출합니다
	 * @return 델리게이트 핸들 (나중에 Remove할 때 사용)
	 */
	COMMONGAME_API FDelegateHandle CallAndRegister_OnPlayerControllerSet(FPlayerControllerSetDelegate::FDelegate Delegate);

	/**
	 * 델리게이트를 등록하고, PlayerState가 이미 존재하면 즉시 호출합니다
	 * @return 델리게이트 핸들 (나중에 Remove할 때 사용)
	 */
	COMMONGAME_API FDelegateHandle CallAndRegister_OnPlayerStateSet(FPlayerStateSetDelegate::FDelegate Delegate);

	/**
	 * 델리게이트를 등록하고, Pawn이 이미 존재하면 즉시 호출합니다
	 * @return 델리게이트 핸들 (나중에 Remove할 때 사용)
	 */
	COMMONGAME_API FDelegateHandle CallAndRegister_OnPlayerPawnSet(FPlayerPawnSetDelegate::FDelegate Delegate);
};