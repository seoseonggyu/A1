// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Inventory/A1InventoryCellWidget.h"
#include "Components/Image.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1InventoryCellWidget)

DEFINE_LOG_CATEGORY(A1InventoryCellWidgetLog);

void UA1InventoryCellWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetSlotState(EA1InventorySlotState::Default);
}

void UA1InventoryCellWidget::SetSlotState(EA1InventorySlotState NewState)
{
	CurrentState = NewState;

	if (Image_Valid)
	{
		switch (NewState)
		{
		case EA1InventorySlotState::Valid:
			Image_Valid->SetColorAndOpacity(ValidColor);
			Image_Valid->SetVisibility(ESlateVisibility::HitTestInvisible);
			break;

		case EA1InventorySlotState::Invalid:
			Image_Valid->SetColorAndOpacity(InvalidColor);
			Image_Valid->SetVisibility(ESlateVisibility::HitTestInvisible);
			break;

		default:
			Image_Valid->SetVisibility(ESlateVisibility::Collapsed);
			break;
		}
	}

}
