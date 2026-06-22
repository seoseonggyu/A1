// Copyright Epic Games, Inc. All Rights Reserved.

#include "Utility/CommonUIExtensions.h"
#include "Layout/CommonUIManagerSubsystem.h"
#include "Layout/CommonPrimaryGameLayout.h"
#include "CommonActivatableWidget.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonUIExtensions)

UCommonUIManagerSubsystem* UCommonUIExtensions::GetUIManagerSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<UCommonUIManagerSubsystem>();
}

UCommonPrimaryGameLayout* UCommonUIExtensions::GetRootLayoutForPlayer(const UObject* WorldContextObject, const ULocalPlayer* LocalPlayer)
{
	if (UCommonUIManagerSubsystem* Subsystem = GetUIManagerSubsystem(WorldContextObject))
	{
		return Subsystem->GetRootLayoutForPlayer(LocalPlayer);
	}
	return nullptr;
}

UCommonPrimaryGameLayout* UCommonUIExtensions::GetRootLayoutForPrimaryPlayer(const UObject* WorldContextObject)
{
	if (UCommonUIManagerSubsystem* Subsystem = GetUIManagerSubsystem(WorldContextObject))
	{
		return Subsystem->GetRootLayoutForPrimaryPlayer();
	}
	return nullptr;
}

UCommonPrimaryGameLayout* UCommonUIExtensions::GetRootLayoutForPlayerController(const APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return nullptr;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return nullptr;
	}

	return GetRootLayoutForPlayer(PlayerController, LocalPlayer);
}

UCommonActivatableWidget* UCommonUIExtensions::PushWidgetToLayerForPrimaryPlayer(const UObject* WorldContextObject, FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	if (UCommonPrimaryGameLayout* Layout = GetRootLayoutForPrimaryPlayer(WorldContextObject))
	{
		return Layout->PushWidgetToLayerStack(LayerTag, WidgetClass);
	}
	return nullptr;
}

UCommonActivatableWidget* UCommonUIExtensions::PushWidgetToLayerForPlayer(const ULocalPlayer* LocalPlayer, FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	if (!LocalPlayer)
	{
		return nullptr;
	}

	if (UCommonPrimaryGameLayout* Layout = GetRootLayoutForPlayer(LocalPlayer, LocalPlayer))
	{
		return Layout->PushWidgetToLayerStack(LayerTag, WidgetClass);
	}
	return nullptr;
}
