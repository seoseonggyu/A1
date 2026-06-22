// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "CommonExtensionActivatableWidget.generated.h"

struct FUIInputConfig;

//-----------------------------------------------------------------------------
// ECommonWidgetInputMode
//-----------------------------------------------------------------------------

/**
 * 위젯의 입력 모드 설정
 */
UENUM(BlueprintType)
enum class ECommonWidgetInputMode : uint8
{
	/** 부모 위젯의 설정을 따릅니다 */
	Default,

	/** 게임과 UI 모두 입력을 받습니다 */
	GameAndMenu,

	/** 게임만 입력을 받습니다 (HUD용) */
	Game,

	/** UI만 입력을 받습니다 (메뉴용) */
	Menu
};

//-----------------------------------------------------------------------------
// UCommonExtensionActivatableWidget
//-----------------------------------------------------------------------------

/**
 * 입력 모드 설정 기능이 추가된 활성화 가능 위젯 클래스
 *
 * CommonUI의 UCommonActivatableWidget을 상속하며, 활성화/비활성화 라이프사이클을 제공합니다.
 * 메뉴, 팝업, 모달 등 활성화 상태 관리가 필요한 UI에 사용됩니다.
 */
UCLASS(Abstract, meta = (DisableNativeTick))
class COMMONUIEXTENSION_API UCommonExtensionActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UCommonExtensionActivatableWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//-----------------------------------------------------------------------------
	// UCommonActivatableWidget 오버라이드
	//-----------------------------------------------------------------------------

	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

protected:
	/** 이 위젯이 활성화될 때 사용할 입력 모드 */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	ECommonWidgetInputMode InputConfig = ECommonWidgetInputMode::Default;

	/** Game 입력 모드에서 마우스 캡처 동작 */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently;
};