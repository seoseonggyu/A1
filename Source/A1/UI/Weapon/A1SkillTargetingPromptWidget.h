// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widget/CommonExtensionUserWidget.h"
#include "A1SkillTargetingPromptWidget.generated.h"

/**
 * UA1SkillTargetingPromptWidget
 *
 * AOE 스킬 조준 중(좌클릭 발동 / 우클릭 취소)임을 안내하는 상시 존재 위젯.
 * UA1RangedChargeWidget과 같은 방식으로 HUD에 미리 배치해두고 평소엔 숨겨둔다.
 * 안내 문구 자체는 BP에서 TextBlock으로 구성한다.
 */
UCLASS(Abstract)
class A1_API UA1SkillTargetingPromptWidget : public UCommonExtensionUserWidget
{
	GENERATED_BODY()

public:
	/** 위젯을 보이게 한다. */
	void ShowPrompt();

	/** 위젯을 다시 숨긴다. */
	void HidePrompt();

protected:
	virtual void NativeConstruct() override;
};
