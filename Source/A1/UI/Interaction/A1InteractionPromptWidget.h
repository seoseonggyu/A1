// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widget/CommonExtensionUserWidget.h"
#include "A1InteractionPromptWidget.generated.h"

class UTextBlock;

DECLARE_LOG_CATEGORY_EXTERN(A1InteractionPromptWidgetLog, Log, All);

/**
 * UA1InteractionPromptWidget
 *
 * 상호작용 대상(문/픽업 등) 근처에 떠 있는 "줍기" 류 프롬프트 위젯.
 * AA1WorldInteractable의 WidgetComponent에 꽂혀 월드 스페이스로 렌더링된다.
 * 문구 채우기는 SetPromptText로 제어하고, 시각 요소는 BP에서 디자인한다.
 */
UCLASS(Abstract)
class A1_API UA1InteractionPromptWidget : public UCommonExtensionUserWidget
{
	GENERATED_BODY()

public:
	/** 프롬프트 문구를 채운다. (예: "줍기") */
	void SetPromptText(const FText& InText);

protected:
	//-----------------------------------------------------------------------------
	// BindWidget (BP에서 배치)
	//-----------------------------------------------------------------------------

	/** 프롬프트 문구 표시 텍스트. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Prompt;
};
