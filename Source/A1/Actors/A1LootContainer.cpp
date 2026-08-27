// Copyright Epic Games, Inc. All Rights Reserved.

#include "A1LootContainer.h"

#include "A1GameplayTags.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Inventory/InventoryComponent.h"
#include "UI/Interaction/A1InteractionPromptWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1LootContainer)

DEFINE_LOG_CATEGORY(A1LootContainerLog);

AA1LootContainer::AA1LootContainer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	// 커서 상호작용 트레이스에 걸리도록 AA1WorldInteractable과 동일한 프로파일을 사용한다.
	Mesh->SetCollisionProfileName(TEXT("A1Interactable"));

	PromptWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("PromptWidgetComponent"));
	PromptWidgetComponent->SetupAttachment(Mesh);
	PromptWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	PromptWidgetComponent->SetDrawAtDesiredSize(true);
	PromptWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PromptWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	PromptWidgetComponent->SetVisibility(false);

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

	InteractionTitle = NSLOCTEXT("A1Interaction", "LootContainer", "루팅하기");
}

void AA1LootContainer::GetHighlightComponents(TArray<UPrimitiveComponent*>& OutComponents) const
{
	if (Mesh)
	{
		OutComponents.Add(Mesh);
	}
}

void AA1LootContainer::GatherInteractionOptions(const FA1InteractionQuery& Query, TArray<FA1InteractionOption>& OutOptions) const
{
	FA1InteractionOption Option;
	Option.Interactable = TScriptInterface<IA1Interactable>(const_cast<AA1LootContainer*>(this));
	Option.Title = InteractionTitle;
	Option.InteractionRange = InteractionRange;
	Option.InteractEventTag = A1GameplayTags::GameplayEvent_Interact_LootContainer;
	// 사용 중인 값: WorldInteractable 기본=1, Door=2, Pickup=3, Corpse=4, Character=5, ExtractionZone=6, LootContainer=7.
	Option.HighlightStencil = 7;
	OutOptions.Add(Option);
}

bool AA1LootContainer::CanInteract(const FA1InteractionQuery& Query) const
{
	return true;
}

void AA1LootContainer::SetInteractionPromptVisible(bool bVisible)
{
	if (PromptWidgetComponent == nullptr)
	{
		return;
	}

	PromptWidgetComponent->SetVisibility(bVisible, true);

	if (bVisible)
	{
		if (UA1InteractionPromptWidget* PromptWidget = Cast<UA1InteractionPromptWidget>(PromptWidgetComponent->GetWidget()))
		{
			PromptWidget->SetPromptText(InteractionTitle);
		}
	}
}

void AA1LootContainer::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		GenerateRandomLootAuthCoroutine();
	}
}

TCoroTask<void> AA1LootContainer::GenerateRandomLootAuthCoroutine()
{
	if (InventoryComponent == nullptr || LootTable.Num() == 0)
	{
		co_return;
	}

	float TotalWeight = 0.f;
	for (const FA1LootTableEntry& Entry : LootTable)
	{
		TotalWeight += FMath::Max(Entry.Weight, 0.f);
	}

	if (TotalWeight <= 0.f)
	{
		co_return;
	}

	const int32 RollCount = FMath::RandRange(MinRolls, MaxRolls);
	for (int32 RollIndex = 0; RollIndex < RollCount; ++RollIndex)
	{
		float Roll = FMath::FRandRange(0.f, TotalWeight);
		const FA1LootTableEntry* Picked = nullptr;
		for (const FA1LootTableEntry& Entry : LootTable)
		{
			Roll -= FMath::Max(Entry.Weight, 0.f);
			if (Roll <= 0.f)
			{
				Picked = &Entry;
				break;
			}
		}

		if (Picked == nullptr || Picked->ItemDefinition == nullptr)
		{
			continue;
		}

		const int32 Count = FMath::RandRange(Picked->MinCount, Picked->MaxCount);
		if (co_await InventoryComponent->AddItemAuthCoroutine(Picked->ItemDefinition, Count) == nullptr)
		{
		}
	}
}
