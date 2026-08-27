// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widget/CommonExtensionUserWidget.h"
#include "A1InteractionHoldWidget.generated.h"

class UWidgetAnimation;

/**
 * UA1InteractionHoldWidget
 *
 * 홀드형 상호작용(탈출, 루팅 상자 등) 중에 화면에 표시하는 진행 표시 위젯.
 * HUD 안에 미리 하나 배치해두고 평소엔 숨겨두는 "상시 존재" 위젯이다 - Push/Pop 스택 방식은
 * 홀드 중 반복적으로 입력 모드를 재계산시켜(RefreshInputConfig) 입력이 흔들리는 부작용이 있어서
 * 쓰지 않는다. UA1Ability_Interact_Hold가 UCommonPrimaryGameLayout::FindWidgetOfType로 이
 * 인스턴스를 찾아 StartHold/StopHold만 호출한다.
 *
 * 진행 표시 자체는 BP에서 ProgressBar 등을 애니메이션(HoldAnim)으로 0→1로 채우도록 구성하고,
 * C++은 그 애니메이션을 홀드 시간에 맞는 재생 속도로 재생만 시킨다 - 애니메이션 실제 길이가
 * 몇 초로 만들어졌든 StartHold에 넘긴 시간에 맞춰 늘어나거나 줄어든다.
 */
UCLASS(Abstract)
class A1_API UA1InteractionHoldWidget : public UCommonExtensionUserWidget
{
	GENERATED_BODY()

public:
	/** 위젯을 보이게 하고, HoldDurationSeconds에 맞춘 재생 속도로 HoldAnim을 처음부터 재생한다. */
	void StartHold(float HoldDurationSeconds);

	/** 애니메이션을 멈추고 위젯을 다시 숨긴다. */
	void StopHold();

protected:
	virtual void NativeConstruct() override;

	//-----------------------------------------------------------------------------
	// BindWidgetAnim (BP에서 구성)
	//-----------------------------------------------------------------------------

	/** 진행 표시 애니메이션(예: ProgressBar Percent 0→1). 실제 길이는 자유롭게 잡아도 된다. */
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> HoldAnim;
};
