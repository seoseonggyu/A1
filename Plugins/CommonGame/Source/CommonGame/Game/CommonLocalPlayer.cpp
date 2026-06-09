// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonLocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonLocalPlayer)

UCommonLocalPlayer::UCommonLocalPlayer()
{
}

FDelegateHandle UCommonLocalPlayer::CallAndRegister_OnPlayerControllerSet(FPlayerControllerSetDelegate::FDelegate Delegate)
{
	if (APlayerController* PC = GetPlayerController(GetWorld()))
	{
		Delegate.Execute(this, PC);
	}

	return OnPlayerControllerSet.Add(Delegate);
}

FDelegateHandle UCommonLocalPlayer::CallAndRegister_OnPlayerStateSet(FPlayerStateSetDelegate::FDelegate Delegate)
{
	const APlayerController* PC = GetPlayerController(GetWorld());

	if (APlayerState* PS = PC ? PC->PlayerState : nullptr)
	{
		Delegate.Execute(this, PS);
	}

	return OnPlayerStateSet.Add(Delegate);
}

FDelegateHandle UCommonLocalPlayer::CallAndRegister_OnPlayerPawnSet(FPlayerPawnSetDelegate::FDelegate Delegate)
{
	const APlayerController* PC = GetPlayerController(GetWorld());

	if (APawn* Pawn = PC ? PC->GetPawn() : nullptr)
	{
		Delegate.Execute(this, Pawn);
	}

	return OnPlayerPawnSet.Add(Delegate);
}