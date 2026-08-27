// Copyright Epic Games, Inc. All Rights Reserved.

#include "A1Ability_Interact_LootContainer.h"

#include "A1GameplayTags.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Layout/CommonPrimaryGameLayout.h"
#include "UI/Loot/A1LootContainerWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_Interact_LootContainer)

DEFINE_LOG_CATEGORY(A1AbilityInteractLootContainerLog);

UA1Ability_Interact_LootContainer::UA1Ability_Interact_LootContainer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	HoldDuration = 5.f;

	SetAssetTags(FGameplayTagContainer(A1GameplayTags::Ability_Interact_LootContainer));

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = A1GameplayTags::GameplayEvent_Interact_LootContainer;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UA1Ability_Interact_LootContainer::OnHoldCompletedLocal(AActor* Interactor, AActor* Target)
{
	if (LootWidgetClass == nullptr || Target == nullptr)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetControllerFromActorInfo());
	UCommonPrimaryGameLayout* Layout = PC ? UCommonPrimaryGameLayout::GetPrimaryGameLayout(PC->GetLocalPlayer()) : nullptr;
	if (Layout == nullptr)
	{
		return;
	}

	APawn* TargetPawn = Cast<APawn>(Target);

	Layout->PushWidgetToLayerStack<UA1LootContainerWidget>(LootWidgetLayerTag, LootWidgetClass, [TargetPawn](UA1LootContainerWidget& Widget)
	{
		Widget.InitializeLoot(TargetPawn);
	});
}
