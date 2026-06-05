// Copyright Epic Games, Inc. All Rights Reserved.

#include "Extension/Condition/ExtensionCondition_HasInputComponent.h"
#include "GameFramework/Pawn.h"
#include "Components/InputComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(ExtensionCondition_HasInputComponent)

bool FExtensionCondition_HasInputComponent::IsSatisfied(AActor* Owner) const
{
	// Pawn 전용 Condition
	APawn* Pawn = Cast<APawn>(Owner);
	if (!Pawn || !Pawn->InputComponent)
	{
		return false;
	}

	// 특정 클래스 타입 확인
	if (RequiredClass)
	{
		return Pawn->InputComponent->IsA(RequiredClass);
	}

	return true;
}
