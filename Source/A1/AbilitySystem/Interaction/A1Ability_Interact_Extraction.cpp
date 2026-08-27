// Copyright Epic Games, Inc. All Rights Reserved.

#include "A1Ability_Interact_Extraction.h"

#include "A1GameplayTags.h"
#include "Interaction/A1Interactable.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_Interact_Extraction)

UA1Ability_Interact_Extraction::UA1Ability_Interact_Extraction(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	HoldDuration = 10.f;

	SetAssetTags(FGameplayTagContainer(A1GameplayTags::Ability_Interact_Extraction));

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = A1GameplayTags::GameplayEvent_Interact_Extraction;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UA1Ability_Interact_Extraction::OnHoldCompletedAuth(AActor* Interactor, AActor* Target)
{
	if (IA1Interactable* Interactable = Cast<IA1Interactable>(Target))
	{
		Interactable->OnInteractAuth(Interactor);
	}
}
