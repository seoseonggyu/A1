// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/A1CharacterMovementComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/A1VitalSet.h"
#include "Player/A1Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1CharacterMovementComponent)

DEFINE_LOG_CATEGORY(A1CharacterMovementComponentLog);

UA1CharacterMovementComponent::UA1CharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

float UA1CharacterMovementComponent::GetMaxSpeed() const
{
	const float BaseSpeed = Super::GetMaxSpeed();

	const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(GetCharacterOwner());
	const UAbilitySystemComponent* ASC = AbilitySystemInterface ? AbilitySystemInterface->GetAbilitySystemComponent() : nullptr;
	if (ASC == nullptr || ASC->HasAttributeSetForAttribute(UA1VitalSet::GetMoveSpeedMultiplierAttribute()) == false)
	{
		return BaseSpeed;
	}

	return BaseSpeed * ASC->GetNumericAttribute(UA1VitalSet::GetMoveSpeedMultiplierAttribute());
}
