// Copyright Epic Games, Inc. All Rights Reserved.

#include "InteractionPromptViewModel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractionPromptViewModel)

DEFINE_LOG_CATEGORY(InteractionPromptViewModelLog);

const FName UInteractionPromptViewModel::ViewModelName = TEXT("InteractionPromptViewModel");

void UInteractionPromptViewModel::ShowPrompt(const FText& InPromptText)
{
	UE_MVVM_SET_PROPERTY_VALUE(PromptText, InPromptText);
	UE_MVVM_SET_PROPERTY_VALUE(bIsVisible, true);
}

void UInteractionPromptViewModel::HidePrompt()
{
	UE_MVVM_SET_PROPERTY_VALUE(bIsVisible, false);
}
