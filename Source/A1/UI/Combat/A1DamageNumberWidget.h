// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widget/CommonExtensionUserWidget.h"
#include "A1DamageNumberWidget.generated.h"

class UTextBlock;

DECLARE_LOG_CATEGORY_EXTERN(A1DamageNumberWidgetLog, Log, All);

/**
 * UA1DamageNumberWidget
 *
 * 피격 데미지 숫자 표시 위젯. AA1DamageNumberActor의 WidgetComponent에 꽂혀 화면 스페이스로 렌더링된다.
 * 텍스트 채우기와 데미지 비례 크기 계산만 담당하고, 실제 팝업 스케일 트윈은 AA1DamageNumberActor가
 * SetDamageAmount의 반환값(목표 스케일)을 받아 코루틴으로 재생한다. 시각 요소는 BP에서 디자인한다.
 */
UCLASS(Abstract)
class A1_API UA1DamageNumberWidget : public UCommonExtensionUserWidget
{
	GENERATED_BODY()

public:
	/** 데미지 문구를 채우고, 데미지 크기에 비례한 목표 렌더 스케일을 계산해 반환한다. */
	float SetDamageAmount(float DamageAmount);

protected:
	//-----------------------------------------------------------------------------
	// BindWidget (BP에서 배치)
	//-----------------------------------------------------------------------------

	/** 데미지 숫자 표시 텍스트. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Damage;

	//-----------------------------------------------------------------------------
	// 크기 스케일링
	//-----------------------------------------------------------------------------

	/** 크기 스케일링 기준이 되는 데미지 범위(X=최소, Y=최대). */
	UPROPERTY(EditDefaultsOnly, Category = "A1|DamageNumber")
	FVector2D DamageRangeForScale = FVector2D(10.f, 200.f);

	/** DamageRangeForScale에 매핑되는 렌더 스케일 범위(X=최소, Y=최대). */
	UPROPERTY(EditDefaultsOnly, Category = "A1|DamageNumber")
	FVector2D ScaleRange = FVector2D(0.8f, 1.8f);
};
