// Copyright Epic Games, Inc. All Rights Reserved.

#include "Layout/CommonUIManagerSubsystem.h"
#include "Layout/CommonUIPolicy.h"
#include "Layout/CommonPrimaryGameLayout.h"
#include "Game/CommonLocalPlayer.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonUIManagerSubsystem)

DEFINE_LOG_CATEGORY(CommonUIManagerSubsystemLog);

UCommonUIManagerSubsystem::UCommonUIManagerSubsystem()
{
}

void UCommonUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	// LocalPlayer 이벤트를 구독합니다
	LocalPlayerAddedHandle = GameInstance->OnLocalPlayerAddedEvent.AddUObject(this, &ThisClass::OnLocalPlayerAdded);
	LocalPlayerRemovedHandle = GameInstance->OnLocalPlayerRemovedEvent.AddUObject(this, &ThisClass::OnLocalPlayerRemoved);

	// 기본 UI Policy를 로드합니다
	if (!DefaultUIPolicyClass.IsNull())
	{
		if (UClass* PolicyClass = DefaultUIPolicyClass.LoadSynchronous())
		{
			SetCurrentUIPolicy(PolicyClass);
		}
		else
		{
			UE_LOG(CommonUIManagerSubsystemLog, Warning, TEXT("PolicyClass가 존재하지 않습니다"));
		}
	}
}

void UCommonUIManagerSubsystem::Deinitialize()
{
	// LocalPlayer 이벤트 구독을 해제합니다
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		GameInstance->OnLocalPlayerAddedEvent.Remove(LocalPlayerAddedHandle);
		GameInstance->OnLocalPlayerRemovedEvent.Remove(LocalPlayerRemovedHandle);
	}

	CurrentPolicy = nullptr;

	Super::Deinitialize();
}

bool UCommonUIManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// 데디케이티드 서버에서는 생성하지 않습니다
	if (UGameInstance* GameInstance = Cast<UGameInstance>(Outer))
	{
		if (GameInstance->IsDedicatedServerInstance())
		{
			return false;
		}

		// 자식 클래스가 존재하면 이 클래스는 생성하지 않습니다 (자식 클래스가 대신 생성됩니다)
		TArray<UClass*> DerivedClasses;
		GetDerivedClasses(GetClass(), DerivedClasses, false);
		if (DerivedClasses.Num() > 0)
		{
			return false;
		}

		return true;
	}

	return false;
}

void UCommonUIManagerSubsystem::SetCurrentUIPolicy(TSubclassOf<UCommonUIPolicy> PolicyClass)
{
	if (!PolicyClass)
	{
		UE_LOG(CommonUIManagerSubsystemLog, Warning, TEXT("UI Policy 클래스가 nullptr입니다"));
		return;
	}

	// 기존 Policy 정리
	if (CurrentPolicy)
	{
		// 기존 플레이어들의 레이아웃을 정리합니다
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			for (ULocalPlayer* LocalPlayer : GameInstance->GetLocalPlayers())
			{
				if (UCommonLocalPlayer* CommonLocalPlayer = Cast<UCommonLocalPlayer>(LocalPlayer))
				{
					CurrentPolicy->NotifyPlayerRemoved(CommonLocalPlayer);
				}
			}
		}
	}

	// 새 Policy 생성
	CurrentPolicy = NewObject<UCommonUIPolicy>(this, PolicyClass);

	// 이미 존재하는 플레이어들에 대해 레이아웃을 생성합니다
	if (CurrentPolicy)
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			for (ULocalPlayer* LocalPlayer : GameInstance->GetLocalPlayers())
			{
				if (UCommonLocalPlayer* CommonLocalPlayer = Cast<UCommonLocalPlayer>(LocalPlayer))
				{
					CurrentPolicy->NotifyPlayerAdded(CommonLocalPlayer);
				}
			}
		}
	}
}

UCommonPrimaryGameLayout* UCommonUIManagerSubsystem::GetRootLayoutForPlayer(const ULocalPlayer* LocalPlayer) const
{
	if (CurrentPolicy)
	{
		if (const UCommonLocalPlayer* CommonLP = Cast<UCommonLocalPlayer>(LocalPlayer))
		{
			return CurrentPolicy->GetRootLayout(CommonLP);
		}
	}

	return nullptr;
}

UCommonPrimaryGameLayout* UCommonUIManagerSubsystem::GetRootLayoutForPrimaryPlayer() const
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (const UCommonLocalPlayer* PrimaryPlayer = Cast<UCommonLocalPlayer>(GameInstance->GetFirstGamePlayer()))
		{
			return GetRootLayoutForPlayer(PrimaryPlayer);
		}
	}

	return nullptr;
}

void UCommonUIManagerSubsystem::OnLocalPlayerAdded(ULocalPlayer* LocalPlayer) const
{
	if (CurrentPolicy)
	{
		if (UCommonLocalPlayer* CommonLocalPlayer = Cast<UCommonLocalPlayer>(LocalPlayer))
		{
			CurrentPolicy->NotifyPlayerAdded(CommonLocalPlayer);
		}
		else
		{
			UE_LOG(CommonUIManagerSubsystemLog, Warning, TEXT("LocalPlayer가 UCommonLocalPlayer 타입이 아닙니다. DefaultEngine.ini에서 LocalPlayerClass를 설정하세요."));
		}
	}
}

void UCommonUIManagerSubsystem::OnLocalPlayerRemoved(ULocalPlayer* LocalPlayer) const
{
	if (CurrentPolicy)
	{
		if (UCommonLocalPlayer* CommonLocalPlayer = Cast<UCommonLocalPlayer>(LocalPlayer))
		{
			CurrentPolicy->NotifyPlayerRemoved(CommonLocalPlayer);
		}
	}
}