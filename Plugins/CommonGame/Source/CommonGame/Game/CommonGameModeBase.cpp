// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/CommonGameModeBase.h"
#include "Game/CommonCharacter.h"
#include "Game/CommonGameStateBase.h"
#include "Game/CommonHUD.h"
#include "Game/CommonPlayerController.h"
#include "Game/CommonPlayerState.h"
#include "Game/CommonWorldSettings.h"
#include "Experience/ExperienceDefinition.h"
#include "Experience/ExperienceManagerComponent.h"
#include "Awaiters/Time.h"
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
	// GameState 생성까지 대기합니다
	co_await Coro::Latent::Until(this, [this]() { return GetGameState<ACommonGameStateBase>() != nullptr; });

	// WorldSettings에서 Experience 가져오기
	ACommonWorldSettings* CommonWorldSettings = Cast<ACommonWorldSettings>(GetWorld()->GetWorldSettings());
	if (!CommonWorldSettings)
	{
		UE_LOG(CommonGameModeLog, Error, TEXT("CommonWorldSettings를 찾을 수 없습니다"));
		co_return;
	}

	FPrimaryAssetId ExperienceId = CommonWorldSettings->GetDefaultExperienceId();
	if (!ExperienceId.IsValid())
	{
		UE_LOG(CommonGameModeLog, Error, TEXT("WorldSettings에 DefaultExperienceId가 설정되지 않았습니다"));
		co_return;
	}

	ACommonGameStateBase* CommonGameState = GetGameState<ACommonGameStateBase>();
	UExperienceManagerComponent* ExperienceManager = CommonGameState->GetExperienceManagerComponent();
	if (!ExperienceManager)
	{
		UE_LOG(CommonGameModeLog, Error, TEXT("ExperienceManagerComponent를 찾을 수 없습니다"));
		co_return;
	}

	// Experience 설정 시작
	UE_LOG(CommonGameModeLog, Log, TEXT("Experience 로드 시작: %s"), *ExperienceId.ToString());
	ExperienceManager->SetCurrentExperienceAuth(ExperienceId);


	// Experience 로드 완료 대기 (델리게이트 기반)
	if (const UExperienceDefinition* Experience = co_await UExperienceManagerComponent::WaitForExperienceLoaded_HighStaticCoroutine(this))
	{
		OnExperienceLoaded(Experience);
	}

}


void ACommonGameModeBase::OnExperienceLoaded(const UExperienceDefinition* Experience)
{
	UE_LOG(CommonGameModeLog, Log, TEXT("Experience 로드 완료: %s"), *Experience->GetName());

	// Experience 로드 전에 접속한 플레이어들 스폰 시도
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Iterator->Get();
		if (PC && PC->GetPawn() == nullptr)
		{
			if (PlayerCanRestart(PC))
			{
				RestartPlayer(PC);
			}
		}
	}

}

void ACommonGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	// Experience가 로드되지 않았으면 처리하지 않습니다
	if (!IsExperienceLoaded())
	{
		UE_LOG(CommonGameModeLog, Log, TEXT("Experience 로드 대기 중, 플레이어 처리 지연: %s"), *GetNameSafe(NewPlayer));
		return;
	}

	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
}

UClass* ACommonGameModeBase::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	// Experience가 로드되지 않았으면 nullptr 반환 (스폰 대기)
	if (!IsExperienceLoaded())
	{
		return nullptr;
	}

	if (ACommonGameStateBase* CommonGameState = GetGameState<ACommonGameStateBase>())
	{
		if (UExperienceManagerComponent* ExperienceManager = CommonGameState->GetExperienceManagerComponent())
		{
			if (const UExperienceDefinition* Experience = ExperienceManager->GetCurrentExperienceChecked())
			{
				if (Experience->DefaultPawnClass)
				{
					return Experience->DefaultPawnClass;
				}
			}
		}
	}

	UE_LOG(CommonGameModeLog, Warning, TEXT("Experience에서 DefaultPawnClass를 가져오지 못함, 기본 클래스 사용: %s"), *GetNameSafe(InController));
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

APawn* ACommonGameModeBase::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{	
	// Experience가 로드되지 않았으면 스폰하지 않습니다
	if (!IsExperienceLoaded())
	{
		UE_LOG(CommonGameModeLog, Log, TEXT("Experience 로드 대기 중, Pawn 스폰 지연: %s"), *GetNameSafe(NewPlayer));
		return nullptr;
	}

	return Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, SpawnTransform);
}

bool ACommonGameModeBase::IsExperienceLoaded() const
{
	if (ACommonGameStateBase* CommonGameState = GetGameState<ACommonGameStateBase>())
	{
		if (UExperienceManagerComponent* ExperienceManager = CommonGameState->GetExperienceManagerComponent())
		{
			return ExperienceManager->IsExperienceLoaded();
		}
	}

	return false;
}

