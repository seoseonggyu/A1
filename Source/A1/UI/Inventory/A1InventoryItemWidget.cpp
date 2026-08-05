// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Inventory/A1InventoryItemWidget.h"
#include "UI/Inventory/A1InventoryWidget.h"
#include "UI/Inventory/A1ItemDragDrop.h"

#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemInstance.h"
#include "Inventory/ItemDefinition.h"

#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1InventoryItemWidget)

DEFINE_LOG_CATEGORY(A1InventoryItemWidgetLog);

int32 UA1InventoryItemWidget::GetItemId() const
{
	return ItemInstance ? ItemInstance->ItemId : INDEX_NONE;
}

void UA1InventoryItemWidget::InitializeItem(UA1InventoryWidget* InOwnerInventory, UItemInstance* InItemInstance, int32 InStackCount, FVector2D InUnitCellSize)
{
	if (!InOwnerInventory || !InItemInstance)
	{
		return;
	}

	OwnerInventory = InOwnerInventory;
	ItemInstance = InItemInstance;
	StackCount = InStackCount;
	UnitCellSize = InUnitCellSize;

	// 아이템이 차지하는 픽셀 크기 = SlotCount * 한 칸 크기
	const FIntPoint SlotCount = UInventoryComponent::GetSizeFromDefinition(InItemInstance->Definition);
	const FVector2D WidgetSize(SlotCount.X * UnitCellSize.X, SlotCount.Y * UnitCellSize.Y);

	if (SizeBox_Root)
	{
		SizeBox_Root->SetWidthOverride(WidgetSize.X);
		SizeBox_Root->SetHeightOverride(WidgetSize.Y);
	}

	// 아이콘
	if (Image_Icon && InItemInstance->Definition && InItemInstance->Definition->Icon)
	{
		Image_Icon->SetBrushFromTexture(InItemInstance->Definition->Icon, true);
	}

	RefreshStackCount(InStackCount);

	OnItemRefreshed(InItemInstance, InStackCount);
}

void UA1InventoryItemWidget::RefreshStackCount(int32 InStackCount)
{
	StackCount = InStackCount;

	if (Text_Count)
	{
		// 1개 이하면 수량 숨김
		Text_Count->SetText(InStackCount <= 1 ? FText::GetEmpty() : FText::AsNumber(InStackCount));
	}
}

void UA1InventoryItemWidget::SetDragVisualOpacity(bool bDragging)
{
	SetRenderOpacity(bDragging ? 0.5f : 1.f);
}
