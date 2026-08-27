// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Loot/A1LootContainerWidget.h"
#include "UI/Inventory/A1InventoryWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1LootContainerWidget)

void UA1LootContainerWidget::InitializeLoot(APawn* InTargetPawn)
{
	if (InventoryWidget_Other)
	{
		InventoryWidget_Other->SetTargetPawnOverride(InTargetPawn, /*bInReadOnly=*/ false);
	}
}
