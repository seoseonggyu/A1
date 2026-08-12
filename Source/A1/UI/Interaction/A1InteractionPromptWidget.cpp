// Copyright Epic Games, Inc. All Rights Reserved.

#include "A1InteractionPromptWidget.h"

#include "Components/TextBlock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1InteractionPromptWidget)

DEFINE_LOG_CATEGORY(A1InteractionPromptWidgetLog);

void UA1InteractionPromptWidget::SetPromptText(const FText& InText)
{
	if (Text_Prompt != nullptr)
	{
		Text_Prompt->SetText(InText);
	}

}
