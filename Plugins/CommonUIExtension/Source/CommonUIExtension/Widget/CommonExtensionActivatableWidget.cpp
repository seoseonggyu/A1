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
	// 탑다운 커서 게임: 뷰포트가 마우스를 캡처(좌클릭 홀드 등)해도 커서를 숨기지 않는다.
	// (기본값 true면 클릭/차징 중 커서가 사라진다)
	case ECommonWidgetInputMode::GameAndMenu:
		return FUIInputConfig(ECommonInputMode::All, GameMouseCaptureMode, /*bHideCursorDuringViewportCapture=*/false);
	case ECommonWidgetInputMode::Game:
		return FUIInputConfig(ECommonInputMode::Game, GameMouseCaptureMode, /*bHideCursorDuringViewportCapture=*/false);
	case ECommonWidgetInputMode::Menu:
		return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
	case ECommonWidgetInputMode::Default:
	default:
		return TOptional<FUIInputConfig>();
	}
}