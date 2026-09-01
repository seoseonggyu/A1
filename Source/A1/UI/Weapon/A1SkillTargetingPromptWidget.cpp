// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Weapon/A1SkillTargetingPromptWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1SkillTargetingPromptWidget)

void UA1SkillTargetingPromptWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 평소엔 숨김. BP 디자이너 기본값에 기대지 않고 항상 확실히 숨겨진 상태로 시작한다.
	SetVisibility(ESlateVisibility::Collapsed);
}

void UA1SkillTargetingPromptWidget::ShowPrompt()
{
	// 입력을 가로채면 안 되므로 SelfHitTestInvisible (마우스/입력은 그대로 아래로 통과).
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UA1SkillTargetingPromptWidget::HidePrompt()
{
	SetVisibility(ESlateVisibility::Collapsed);
}
