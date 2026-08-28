// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widget/CommonExtensionUserWidget.h"
#include "A1RangedChargeWidget.generated.h"

class UWidgetAnimation;

/**
 * UA1RangedChargeWidget
 *
 * 원거리 무기(지팡이 등)로 차징 중임을 표시하는 위젯. UA1InteractionHoldWidget과 같은 방식으로
 * HUD 안에 미리 하나 배치해두고 평소엔 숨겨두는 "상시 존재" 위젯이다(Push/Pop 스택 방식은 쓰지 않음).
 * UA1Ability_RangedWeaponAttack이 UCommonPrimaryGameLayout::FindWidgetOfType로 이 인스턴스를
 * 찾아 StartCharge/StopCharge만 호출한다.
 *
 * 진행 표시는 BP에서 ProgressBar 등을 애니메이션(ChargeAnim)으로 0→1로 채우도록 구성한다.
 * MaxChargeDuration보다 오래 홀드해도(최대 충전 이후 계속 누르고 있어도) 애니메이션은 재생이 끝난
 * 마지막 프레임(가득 찬 상태)에 그대로 머무르므로 별도 처리가 필요 없다.
 */
UCLASS(Abstract)
class A1_API UA1RangedChargeWidget : public UCommonExtensionUserWidget
{
	GENERATED_BODY()

public:
	/** 위젯을 보이게 하고, MaxChargeDurationSeconds에 맞춘 재생 속도로 ChargeAnim을 처음부터 재생한다. */
	void StartCharge(float MaxChargeDurationSeconds);

	/** 애니메이션을 멈추고 위젯을 다시 숨긴다. */
	void StopCharge();

protected:
	virtual void NativeConstruct() override;

	//-----------------------------------------------------------------------------
	// BindWidgetAnim (BP에서 구성)
	//-----------------------------------------------------------------------------

	/** 충전 진행 표시 애니메이션(예: ProgressBar Percent 0→1). 실제 길이는 자유롭게 잡아도 된다. */
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> ChargeAnim;
};
