// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Inventory/A1InventoryWidget.h"
#include "UI/Inventory/A1InventoryCellWidget.h"
#include "UI/Inventory/A1InventoryItemWidget.h"
#include "UI/Inventory/A1ItemDragDrop.h"

#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemInstance.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1InventoryWidget)

DEFINE_LOG_CATEGORY(A1InventoryWidgetLog);

UA1InventoryWidget::UA1InventoryWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

//-----------------------------------------------------------------------------
// 활성화 / 비활성화
//-----------------------------------------------------------------------------

void UA1InventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetupInventory();
}

void UA1InventoryWidget::NativeDestruct()
{
	TearDown();

	Super::NativeDestruct();
}

void UA1InventoryWidget::SetupInventory()
{
	if (InventoryComponent)
	{
		return;
	}

	InventoryComponent = UInventoryComponent::FindInventoryComponent(GetOwningPlayer());
	if (!InventoryComponent)
	{
		UE_LOG(A1InventoryWidgetLog, Warning, TEXT("SetupInventory: InventoryComponent를 찾을 수 없습니다"));
		return;
	}

	CachedGridSize = InventoryComponent->GetGridSize();

	BuildCells();
	SpawnExistingItems();

	GridChangedHandle = InventoryComponent->OnInventoryGridChanged.AddUObject(this, &ThisClass::HandleGridChanged);
}

void UA1InventoryWidget::TearDown()
{
	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryGridChanged.Remove(GridChangedHandle);
	}
	GridChangedHandle.Reset();

	if (CanvasPanel_Items)
	{
		CanvasPanel_Items->ClearChildren();
	}
	ItemWidgets.Reset();
	DropPreviewCellWidgets.Reset();

	if (GridPanel_Cells)
	{
		GridPanel_Cells->ClearChildren();
	}
	CellWidgets.Reset();

	InventoryComponent = nullptr;
	CachedGridSize = FIntPoint::ZeroValue;
	PrevPreviewAnchor = FIntPoint(-999, -999);
}

//-----------------------------------------------------------------------------
// 그리드 구성
//-----------------------------------------------------------------------------

void UA1InventoryWidget::BuildCells()
{
	if (!GridPanel_Cells || !CellWidgetClass)
	{
		UE_LOG(A1InventoryWidgetLog, Warning, TEXT("BuildCells: GridPanel_Cells 또는 CellWidgetClass가 없습니다"));
		return;
	}

	GridPanel_Cells->ClearChildren();
	CellWidgets.Reset();
	CellWidgets.SetNum(CachedGridSize.X * CachedGridSize.Y);

	for (int32 Y = 0; Y < CachedGridSize.Y; ++Y)
	{
		for (int32 X = 0; X < CachedGridSize.X; ++X)
		{
			UA1InventoryCellWidget* Cell = CreateWidget<UA1InventoryCellWidget>(GetOwningPlayer(), CellWidgetClass);
			if (!Cell)
			{
				continue;
			}

			CellWidgets[CellIndex(X, Y)] = Cell;
			GridPanel_Cells->AddChildToUniformGrid(Cell, Y, X);
		}
	}
}

void UA1InventoryWidget::SpawnExistingItems()
{
	if (!InventoryComponent)
	{
		return;
	}

	TArray<UItemInstance*> Items;
	InventoryComponent->GetAllItems(Items);

	for (UItemInstance* Item : Items)
	{
		if (!Item)
		{
			continue;
		}

		const FIntPoint SlotPos = InventoryComponent->GetSlotPosition(Item);
		if (SlotPos.X < 0)
		{
			// 그리드에 배치되지 않은 아이템(예: 미배치)은 건너뜀
			continue;
		}

		int32 StackCount = 1;
		if (const FInventoryEntry* Entry = InventoryComponent->FindEntry(Item))
		{
			StackCount = Entry->StackCount;
		}

		PlaceItemWidget(Item->ItemId, Item, SlotPos, StackCount);
	}
}

//-----------------------------------------------------------------------------
// 델리게이트 콜백
//-----------------------------------------------------------------------------

void UA1InventoryWidget::HandleGridChanged(EInventoryGridChangeType ChangeType, int32 ItemId, UItemInstance* Instance, const FIntPoint& SlotPos, int32 StackCount)
{
	switch (ChangeType)
	{
	case EInventoryGridChangeType::Added:
	case EInventoryGridChangeType::Changed:
	case EInventoryGridChangeType::Moved:
		if (SlotPos.X < 0 || SlotPos.Y < 0)
		{
			// 그리드에서 내려진(미배치) 아이템 → 예: 장착되어 인벤토리 UI에서 제거
			RemoveItemWidget(ItemId);
		}
		else
		{
			PlaceItemWidget(ItemId, Instance, SlotPos, StackCount);
		}
		break;

	case EInventoryGridChangeType::Removed:
		RemoveItemWidget(ItemId);
		break;
	}
}

void UA1InventoryWidget::PlaceItemWidget(int32 ItemId, UItemInstance* Instance, const FIntPoint& SlotPos, int32 StackCount)
{
	if (!CanvasPanel_Items || !Instance || SlotPos.X < 0)
	{
		return;
	}

	UA1InventoryItemWidget* ItemWidget = nullptr;
	if (TObjectPtr<UA1InventoryItemWidget>* Found = ItemWidgets.Find(ItemId))
	{
		ItemWidget = *Found;
	}

	// 없으면 새로 생성
	if (!ItemWidget)
	{
		if (!ItemWidgetClass)
		{
			UE_LOG(A1InventoryWidgetLog, Warning, TEXT("PlaceItemWidget: ItemWidgetClass가 없습니다"));
			return;
		}

		ItemWidget = CreateWidget<UA1InventoryItemWidget>(GetOwningPlayer(), ItemWidgetClass);
		if (!ItemWidget)
		{
			return;
		}

		ItemWidgets.Add(ItemId, ItemWidget);
		CanvasPanel_Items->AddChildToCanvas(ItemWidget);
		ItemWidget->InitializeItem(this, Instance, StackCount, UnitCellSize);
	}
	else
	{
		ItemWidget->RefreshStackCount(StackCount);
	}

	// 캔버스 위치 = 앵커 셀 * 한 칸 크기
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(ItemWidget->Slot))
	{
		CanvasSlot->SetAutoSize(true);
		CanvasSlot->SetPosition(FVector2D(SlotPos.X * UnitCellSize.X, SlotPos.Y * UnitCellSize.Y));
	}

	// 드래그 후 남아있을 수 있는 반투명 복구
	ItemWidget->SetDragVisualOpacity(false);
}

void UA1InventoryWidget::RemoveItemWidget(int32 ItemId)
{
	TObjectPtr<UA1InventoryItemWidget> ItemWidget = nullptr;
	if (ItemWidgets.RemoveAndCopyValue(ItemId, ItemWidget) && ItemWidget && CanvasPanel_Items)
	{
		CanvasPanel_Items->RemoveChild(ItemWidget);
	}
}

//-----------------------------------------------------------------------------
// 드래그 & 드롭
//-----------------------------------------------------------------------------

bool UA1InventoryWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UA1ItemDragDrop* DragOp = Cast<UA1ItemDragDrop>(InOperation);
	if (!DragOp || !DragOp->ItemInstance || !InventoryComponent)
	{
		return false;
	}

	const FIntPoint Anchor = ComputeDropAnchor(InGeometry, InDragDropEvent, DragOp->GrabOffset);
	if (Anchor == PrevPreviewAnchor)
	{
		return true;
	}

	const FIntPoint Size = UInventoryComponent::GetSizeFromDefinition(DragOp->ItemInstance->Definition);
	const bool bCanPlace = InventoryComponent->CanPlaceAt(Anchor, Size, DragOp->ItemInstance);

	UpdateDropPreview(Anchor, Size, bCanPlace);

	return true;
}

void UA1InventoryWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);

	ClearDropPreview();
}

bool UA1InventoryWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UA1ItemDragDrop* DragOp = Cast<UA1ItemDragDrop>(InOperation);

	ClearDropPreview();

	if (!DragOp || !DragOp->ItemInstance || !InventoryComponent)
	{
		return false;
	}

	const FIntPoint Anchor = ComputeDropAnchor(InGeometry, InDragDropEvent, DragOp->GrabOffset);
	const FIntPoint Size = UInventoryComponent::GetSizeFromDefinition(DragOp->ItemInstance->Definition);

	if (!InventoryComponent->CanPlaceAt(Anchor, Size, DragOp->ItemInstance))
	{
		// 배치 불가 위치에 놓음 -> Unhandled 반환으로 NativeOnDragCancelled가 원본 위젯을 원상 복구하게 한다
		return false;
	}

	// 실제 이동은 서버 권위로 검증 후 처리되고, 결과가 리플리케이션되면 위젯이 갱신된다.
	if (DragOp->FromEquipmentSlotTag.IsValid())
	{
		// 장비창에서 온 드래그: 서버에서 장착 해제 후 인벤토리 그리드에 배치
		//InventoryComponent->UnequipToInventoryServer(DragOp->ItemId, DragOp->FromEquipmentSlotTag, Anchor);
	}
	else
	{
		// 인벤토리 내부 이동
		InventoryComponent->MoveItemServer(DragOp->ItemId, Anchor);
	}

	return true;
}

FIntPoint UA1InventoryWidget::ComputeDropAnchor(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, const FVector2D& GrabOffset) const
{
	if (!CanvasPanel_Items)
	{
		return FIntPoint(-999, -999);
	}

	// SlotPos * UnitCellSize는 CanvasPanel_Items 좌표계 기준이므로, 반드시 같은 좌표계로 변환해야 한다.
	// (InGeometry는 이 위젯 전체 기준이라, 여백/헤더가 있으면 어긋난다)
	const FVector2D LocalMousePos = CanvasPanel_Items->GetCachedGeometry().AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
	const FVector2D ItemTopLeft = LocalMousePos - GrabOffset;

	return FIntPoint(
		FMath::RoundToInt(ItemTopLeft.X / UnitCellSize.X),
		FMath::RoundToInt(ItemTopLeft.Y / UnitCellSize.Y));
}

void UA1InventoryWidget::UpdateDropPreview(const FIntPoint& Anchor, const FIntPoint& Size, bool bCanPlace)
{
	ClearDropPreview();

	if (!CanvasPanel_Items || !CellWidgetClass)
	{
		return;
	}

	const EA1InventorySlotState State = bCanPlace ? EA1InventorySlotState::Valid : EA1InventorySlotState::Invalid;

	for (int32 OffsetY = 0; OffsetY < Size.Y; ++OffsetY)
	{
		for (int32 OffsetX = 0; OffsetX < Size.X; ++OffsetX)
		{
			const int32 X = Anchor.X + OffsetX;
			const int32 Y = Anchor.Y + OffsetY;
			if (X < 0 || X >= CachedGridSize.X || Y < 0 || Y >= CachedGridSize.Y)
			{
				continue;
			}

			// 배경 GridPanel_Cells는 아이템(CanvasPanel_Items)보다 아래 레이어라 이미 놓인 아이템에 가려진다.
			// 아이템과 같은 CanvasPanel_Items에 나중에 추가해 항상 위에 그려지도록 한다.
			UA1InventoryCellWidget* PreviewCell = CreateWidget<UA1InventoryCellWidget>(GetOwningPlayer(), CellWidgetClass);
			if (!PreviewCell)
			{
				continue;
			}

			CanvasPanel_Items->AddChildToCanvas(PreviewCell);
			if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(PreviewCell->Slot))
			{
				CanvasSlot->SetAutoSize(false);
				CanvasSlot->SetSize(UnitCellSize);
				CanvasSlot->SetPosition(FVector2D(X * UnitCellSize.X, Y * UnitCellSize.Y));
			}

			PreviewCell->SetSlotState(State);
			PreviewCell->SetVisibility(ESlateVisibility::HitTestInvisible);

			DropPreviewCellWidgets.Add(PreviewCell);
		}
	}

	PrevPreviewAnchor = Anchor;
}

void UA1InventoryWidget::ClearDropPreview()
{
	if (CanvasPanel_Items)
	{
		for (UA1InventoryCellWidget* PreviewCell : DropPreviewCellWidgets)
		{
			if (PreviewCell)
			{
				CanvasPanel_Items->RemoveChild(PreviewCell);
			}
		}
	}
	DropPreviewCellWidgets.Reset();

	PrevPreviewAnchor = FIntPoint(-999, -999);
}

int32 UA1InventoryWidget::CellIndex(int32 X, int32 Y) const
{
	return Y * CachedGridSize.X + X;
}
