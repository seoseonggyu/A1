// Copyright Epic Games, Inc. All Rights Reserved.

#include "Input/InputTypes.h"
#include "InputAction.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(InputTypes)

FString FInputActionAndTag::ToString() const
{
	return FString::Printf(TEXT("Tag: [%s] -> Action: [%s]"),
		*InputTag.ToString(),
		InputAction ? *InputAction->GetName() : TEXT("None"));
}