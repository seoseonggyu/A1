// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Interaction/A1InteractionHoldWidget.h"
#include "Animation/WidgetAnimation.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1InteractionHoldWidget)

void UA1InteractionHoldWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 평소엔 숨김. BP 디자이너 기본값에 기대지 않고 항상 확실히 숨겨진 상태로 시작한다.
	SetVisibility(ESlateVisibility::Collapsed);
}

void UA1InteractionHoldWidget::StartHold(float HoldDurationSeconds)
{
	if (HoldAnim == nullptr || HoldDurationSeconds <= 0.f)
	{
		return;
	}

	// 입력을 가로채면 안 되므로 SelfHitTestInvisible (마우스/입력은 그대로 아래로 통과).
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	const float AnimLength = HoldAnim->GetEndTime() - HoldAnim->GetStartTime();
	const float PlayRate = (AnimLength > 0.f) ? (AnimLength / HoldDurationSeconds) : 1.f;

	PlayAnimation(HoldAnim, 0.f, 1, EUMGSequencePlayMode::Forward, PlayRate);
}

void UA1InteractionHoldWidget::StopHold()
{
	StopAllAnimations();
	SetVisibility(ESlateVisibility::Collapsed);
}
