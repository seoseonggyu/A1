// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonPlayerController.h"
#include "Game/CommonLocalPlayer.h"
#include "Game/CommonPlayerState.h"
#include "Game/CommonPlayerCameraManager.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonPlayerController)


ACommonPlayerController::ACommonPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerCameraManagerClass = ACommonPlayerCameraManager::StaticClass();
}

void ACommonPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	// LocalPlayer에게 PlayerController가 설정되었음을 알립니다
	if (UCommonLocalPlayer* LP = Cast<UCommonLocalPlayer>(Player))
	{
		LP->OnPlayerControllerSet.Broadcast(LP, this);

		if (PlayerState)
		{
			LP->OnPlayerStateSet.Broadcast(LP, PlayerState);
		}
	}
}

void ACommonPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	// LocalPlayer에게 Pawn이 설정되었음을 알립니다
	if (UCommonLocalPlayer* CommonLP = Cast<UCommonLocalPlayer>(Player))
	{
		CommonLP->OnPlayerPawnSet.Broadcast(CommonLP, InPawn);
	}
}

void ACommonPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	Super::PostProcessInput(DeltaTime, bGamePaused);

	OnPostProcessInput.Broadcast(DeltaTime, bGamePaused);
}

void ACommonPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// 클라이언트에서 PlayerState가 복제되었을 때 LocalPlayer에게 알립니다
	if (PlayerState)
	{
		if (UCommonLocalPlayer* CommonLP = Cast<UCommonLocalPlayer>(Player))
		{
			CommonLP->OnPlayerStateSet.Broadcast(CommonLP, PlayerState);
		}
	}
}

void ACommonPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 서버에서 ASC의 Owner/Avatar 정보를 설정합니다
	if (ACommonPlayerState* PS = GetPlayerState<ACommonPlayerState>())
	{
		if (UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
		{
			ASC->InitAbilityActorInfo(PS, InPawn);
		}
	}

	// LocalPlayer에게 Pawn이 설정되었음을 알립니다
	if (UCommonLocalPlayer* CommonLP = Cast<UCommonLocalPlayer>(Player))
	{
		CommonLP->OnPlayerPawnSet.Broadcast(CommonLP, InPawn);
	}
}

void ACommonPlayerController::OnUnPossess()
{
	// Super 호출 전에 해야 Pawn 참조 가능
	if (APawn* Avatar = GetPawn())
	{
		const APlayerState* ThePlayerState = PlayerState.Get();
		if (IsValid(ThePlayerState))
		{
			if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ThePlayerState))
			{
				if (ASC->GetAvatarActor() == Avatar)
				{
					ASC->SetAvatarActor(nullptr);
				}
			}
		}
	}

	// LocalPlayer에게 Pawn이 해제되었음을 알립니다
	if (UCommonLocalPlayer* CommonLP = Cast<UCommonLocalPlayer>(Player))
	{
		CommonLP->OnPlayerPawnSet.Broadcast(CommonLP, nullptr);
	}

	Super::OnUnPossess();
}
