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
	// 기본값 false라 SetFocus()가 무시됨 - Esc 등 키 입력을 받으려면 반드시 켜야 함
	SetIsFocusable(true);
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

void UA1InventoryWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// 키보드 포커스를 가져와야 NativeOnKeyDown이 호출된다.
	// 포커스가 없으면 Esc 등 입력이 그대로 새어나가 에디터 단축키(PIE 종료 등)로 흘러간다.
	SetFocus();
}

void UA1InventoryWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	OnInventoryWindowClosed.Broadcast();
}

FReply UA1InventoryWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		DeactivateWidget();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UA1InventoryWidget::SetupInventory()
{
	if (InventoryComponent)
	{
		// 이미 구성됨 (재활성화 등)
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
		PlaceItemWidget(ItemId, Instance, SlotPos, StackCount);
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
	Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);

	UA1ItemDragDrop* DragDrop = Cast<UA1ItemDragDrop>(InOperation);
	if (!DragDrop || !InventoryComponent)
	{
		return false;
	}

	const FIntPoint Anchor = CalcAnchorSlot(InDragDropEvent, DragDrop->GrabOffset);
	if (Anchor == PrevPreviewAnchor)
	{
		return true;
	}
	PrevPreviewAnchor = Anchor;

	ResetCellStates();
	PreviewPlacement(DragDrop->ItemId, Anchor);
	return true;
}

void UA1InventoryWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);

	ResetCellStates();
	PrevPreviewAnchor = FIntPoint(-999, -999);
}

bool UA1InventoryWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	UA1ItemDragDrop* DragDrop = Cast<UA1ItemDragDrop>(InOperation);
	if (!DragDrop || !InventoryComponent)
	{
		return false;
	}

	ResetCellStates();
	PrevPreviewAnchor = FIntPoint(-999, -999);

	// 드래그 시작 위젯의 반투명 복구
	if (DragDrop->FromItemWidget)
	{
		DragDrop->FromItemWidget->SetDragVisualOpacity(false);
	}

	const FIntPoint Anchor = CalcAnchorSlot(InDragDropEvent, DragDrop->GrabOffset);

	// 서버에 이동 요청 → 실제 반영은 복제(OnInventoryGridChanged::Moved)로 처리
	InventoryComponent->MoveItemServer(DragDrop->ItemId, Anchor);
	return true;
}

//-----------------------------------------------------------------------------
// 헬퍼
//-----------------------------------------------------------------------------

void UA1InventoryWidget::ResetCellStates()
{
	for (UA1InventoryCellWidget* Cell : CellWidgets)
	{
		if (Cell)
		{
			Cell->SetSlotState(EA1InventorySlotState::Default);
		}
	}
}

FIntPoint UA1InventoryWidget::CalcAnchorSlot(const FDragDropEvent& InDragDropEvent, const FVector2D& GrabOffset) const
{
	const FVector2D LocalMouse = GetItemCanvasGeometry().AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
	const FVector2D TopLeft = LocalMouse - GrabOffset;

	return FIntPoint(
		FMath::FloorToInt(TopLeft.X / UnitCellSize.X),
		FMath::FloorToInt(TopLeft.Y / UnitCellSize.Y));
}

void UA1InventoryWidget::PreviewPlacement(int32 ItemId, const FIntPoint& AnchorSlot)
{
	if (!InventoryComponent)
	{
		return;
	}

	const UItemInstance* Instance = InventoryComponent->FindItemById(ItemId);
	const FIntPoint Size = UInventoryComponent::GetSizeFromDefinition(Instance ? Instance->Definition : nullptr);

	const bool bCanPlace = InventoryComponent->CanPlaceItemAt(ItemId, AnchorSlot);
	const EA1InventorySlotState State = bCanPlace ? EA1InventorySlotState::Valid : EA1InventorySlotState::Invalid;

	for (int32 Y = AnchorSlot.Y; Y < AnchorSlot.Y + Size.Y; ++Y)
	{
		for (int32 X = AnchorSlot.X; X < AnchorSlot.X + Size.X; ++X)
		{
			if (X < 0 || Y < 0 || X >= CachedGridSize.X || Y >= CachedGridSize.Y)
			{
				continue;
			}

			if (UA1InventoryCellWidget* Cell = CellWidgets[CellIndex(X, Y)])
			{
				Cell->SetSlotState(State);
			}
		}
	}
}

const FGeometry& UA1InventoryWidget::GetItemCanvasGeometry() const
{
	return CanvasPanel_Items->GetCachedGeometry();
}

int32 UA1InventoryWidget::CellIndex(int32 X, int32 Y) const
{
	return Y * CachedGridSize.X + X;
}
