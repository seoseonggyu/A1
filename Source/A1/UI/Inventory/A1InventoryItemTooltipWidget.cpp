// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Inventory/A1InventoryItemTooltipWidget.h"

#include "Inventory/ItemInstance.h"
#include "Inventory/ItemDefinition.h"

#include "Components/TextBlock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1InventoryItemTooltipWidget)

DEFINE_LOG_CATEGORY(A1InventoryItemTooltipWidgetLog);

void UA1InventoryItemTooltipWidget::SetItem(UItemInstance* InItemInstance)
{
	if (!InItemInstance || !InItemInstance->Definition)
	{
		return;
	}

	const UItemDefinition* Definition = InItemInstance->Definition;

	if (Text_Name)
	{
		Text_Name->SetText(Definition->DisplayName);
	}

	if (Text_Description)
	{
		Text_Description->SetText(Definition->Description);
	}

	OnItemSet(InItemInstance);
}
