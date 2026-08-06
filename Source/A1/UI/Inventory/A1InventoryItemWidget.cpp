// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Inventory/A1InventoryItemWidget.h"
#include "UI/Inventory/A1InventoryWidget.h"
#include "UI/Inventory/A1InventoryItemTooltipWidget.h"
#include "UI/Inventory/A1ItemDragDrop.h"

#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemInstance.h"
#include "Inventory/ItemDefinition.h"

#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1InventoryItemWidget)

DEFINE_LOG_CATEGORY(A1InventoryItemWidgetLog);

int32 UA1InventoryItemWidget::GetItemId() const
{
	return ItemInstance ? ItemInstance->ItemId : INDEX_NONE;
}

void UA1InventoryItemWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// UMG 툴팁 시스템이 호버 시 HandleGetTooltipWidget을 호출해 툴팁을 생성하도록 바인딩
	ToolTipWidgetDelegate.BindDynamic(this, &UA1InventoryItemWidget::HandleGetTooltipWidget);

}

FReply UA1InventoryItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// 위젯 좌상단 기준 마우스를 잡은 위치를 기억해뒀다가, 드래그 비주얼이 같은 지점에서 잡히도록 사용
		CachedGrabOffset = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());

		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UA1InventoryItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!ItemInstance || !OwnerInventory.IsValid())
	{
		return;
	}

	UA1ItemDragDrop* DragOp = NewObject<UA1ItemDragDrop>(this);
	DragOp->ItemId = GetItemId();
	DragOp->ItemInstance = ItemInstance;
	DragOp->FromInventory = OwnerInventory.Get();
	DragOp->FromItemWidget = this;
	DragOp->FromSlotPos = OwnerInventory->GetInventoryComponent() ? OwnerInventory->GetInventoryComponent()->GetSlotPosition(ItemInstance) : FIntPoint(-1, -1);
	DragOp->GrabOffset = CachedGrabOffset;

	// 마우스를 따라다닐 비주얼: 같은 클래스의 위젯을 하나 더 만들어 동일하게 초기화
	if (UA1InventoryItemWidget* DragVisual = CreateWidget<UA1InventoryItemWidget>(GetOwningPlayer(), GetClass()))
	{
		DragVisual->InitializeItem(OwnerInventory.Get(), ItemInstance, StackCount, UnitCellSize);

		DragOp->DefaultDragVisual = DragVisual;
		// MouseDown 피벗 = 소스 위젯을 잡았던 지점이 드래그 비주얼에서도 그대로 마우스 아래에 오도록 엔진이 자동 계산
		DragOp->Pivot = EDragPivot::MouseDown;
	}

	// 원본 위젯은 인벤토리에 그대로 두고, 드래그 중임을 반투명으로 표시
	SetDragVisualOpacity(true);

	OutOperation = DragOp;
}

void UA1InventoryItemWidget::NativeOnDragCancelled(const FDragDropEvent& InPointerEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InPointerEvent, InOperation);

	// 유효하지 않은 곳에 드롭되어 취소된 경우, 원본 위젯의 반투명을 복구
	SetDragVisualOpacity(false);
}

UWidget* UA1InventoryItemWidget::HandleGetTooltipWidget()
{
	if (!TooltipWidgetClass || !ItemInstance)
	{
		return nullptr;
	}

	UA1InventoryItemTooltipWidget* Tooltip = CreateWidget<UA1InventoryItemTooltipWidget>(this, TooltipWidgetClass);
	if (Tooltip)
	{
		Tooltip->SetItem(ItemInstance);
	}

	return Tooltip;
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
