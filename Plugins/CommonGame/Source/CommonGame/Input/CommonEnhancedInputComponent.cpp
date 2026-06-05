// Copyright Epic Games, Inc. All Rights Reserved.

#include "Input/CommonEnhancedInputComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonEnhancedInputComponent)

DEFINE_LOG_CATEGORY(CommonEnhancedInputComponentLog);

void UCommonEnhancedInputComponent::SetNativeInputActionMappings(const TArray<FInputActionAndTag>& InMappings)
{
	NativeInputActionMappings = InMappings;
}

void UCommonEnhancedInputComponent::SetAbilityInputActionMappings(const TArray<FInputActionAndTag>& InMappings)
{
	AbilityInputActionMappings = InMappings;
}

const UInputAction* UCommonEnhancedInputComponent::FindNativeActionByTag(const FGameplayTag& InputTag) const
{
	for (const FInputActionAndTag& Mapping : NativeInputActionMappings)
	{
		if (Mapping.InputTag.MatchesTagExact(InputTag))
		{
			return Mapping.InputAction.Get();
		}
	}
	return nullptr;
}

const UInputAction* UCommonEnhancedInputComponent::FindAbilityActionByTag(const FGameplayTag& InputTag) const
{
	for (const FInputActionAndTag& Mapping : AbilityInputActionMappings)
	{
		if (Mapping.InputTag.MatchesTagExact(InputTag))
		{
			return Mapping.InputAction.Get();
		}
	}
	return nullptr;
}
