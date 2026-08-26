// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Combat/A1DamageNumberWidget.h"

#include "Components/TextBlock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1DamageNumberWidget)

DEFINE_LOG_CATEGORY(A1DamageNumberWidgetLog);

float UA1DamageNumberWidget::SetDamageAmount(float DamageAmount)
{
	if (Text_Damage)
	{
		Text_Damage->SetText(FText::AsNumber(FMath::RoundToInt(DamageAmount)));
	}

	return FMath::GetMappedRangeValueClamped(DamageRangeForScale, ScaleRange, DamageAmount);
}
