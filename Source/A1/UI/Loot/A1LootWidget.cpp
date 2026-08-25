// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Loot/A1LootWidget.h"
#include "UI/Equipment/A1EquipmentWidget.h"
#include "UI/Inventory/A1InventoryWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1LootWidget)

void UA1LootWidget::InitializeLoot(APawn* InTargetPawn)
{
	if (EquipmentWidget_Other)
	{
		EquipmentWidget_Other->SetTargetPawnOverride(InTargetPawn, /*bInReadOnly=*/ false);
	}

	if (InventoryWidget_Other)
	{
		InventoryWidget_Other->SetTargetPawnOverride(InTargetPawn, /*bInReadOnly=*/ false);
	}
}
