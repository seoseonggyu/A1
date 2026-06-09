// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/CommonAttributeSet.h"
#include "Net/Core/PushModel/PushModel.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonAttributeSet)

UCommonAttributeSet::UCommonAttributeSet()
{
}

void UCommonAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	// PushModel: Attribute 값이 변경된 경우에만 dirty 마킹
	if (OldValue != NewValue)
	{
		if (FProperty* Property = Attribute.GetUProperty())
		{
			MARK_PROPERTY_DIRTY(this, Property);
		}
	}
}

void UCommonAttributeSet::PostNetInit()
{
	// 파생 클래스에서 오버라이드하여 초기화 로직 추가 가능
}