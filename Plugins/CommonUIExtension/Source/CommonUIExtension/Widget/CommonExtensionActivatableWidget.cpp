// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widget/CommonExtensionActivatableWidget.h"
#include "Input/UIActionBindingHandle.h"
#include "CommonInputModeTypes.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonExtensionActivatableWidget)

UCommonExtensionActivatableWidget::UCommonExtensionActivatableWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

TOptional<FUIInputConfig> UCommonExtensionActivatableWidget::GetDesiredInputConfig() const
{
	switch (InputConfig)
	{
	case ECommonWidgetInputMode::GameAndMenu:
		return FUIInputConfig(ECommonInputMode::All, GameMouseCaptureMode);
	case ECommonWidgetInputMode::Game:
		return FUIInputConfig(ECommonInputMode::Game, GameMouseCaptureMode);
	case ECommonWidgetInputMode::Menu:
		return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
	case ECommonWidgetInputMode::Default:
	default:
		return TOptional<FUIInputConfig>();
	}
}