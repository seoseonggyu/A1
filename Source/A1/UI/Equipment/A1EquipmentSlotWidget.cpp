// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Equipment/A1EquipmentSlotWidget.h"
#include "UI/Inventory/A1ItemDragDrop.h"

#include "Equipment/EquipmentInstance.h"
#include "Equipment/EquipmentDefinition.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemInstance.h"
#include "Inventory/ItemDefinition.h"
#include "Inventory/Fragment/ItemFragment_Equipment.h"

#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Blueprint/UserWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1EquipmentSlotWidget)

DEFINE_LOG_CATEGORY(A1EquipmentSlotWidgetLog);

void UA1EquipmentSlotWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetEquipmentInstance(nullptr);
}

FReply UA1EquipmentSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && EquipmentInstance)
	{
		// 위젯 좌상단 기준 마우스를 잡은 위치와 위젯 크기를 기억해뒀다가, 드래그 비주얼이 같은 지점/크기로 잡히도록 사용
		CachedGrabOffset = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
		CachedSlotSize = InGeometry.GetLocalSize();

		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UA1EquipmentSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	UItemInstance* SourceItem = EquipmentInstance ? EquipmentInstance->GetSourceItemInstance() : nullptr;
	if (!SourceItem)
	{
		return;
	}

	UA1ItemDragDrop* DragOp = NewObject<UA1ItemDragDrop>(this);
	DragOp->ItemId = SourceItem->ItemId;
	DragOp->ItemInstance = SourceItem;
	DragOp->FromEquipmentSlotWidget = this;
	DragOp->FromEquipmentSlotTag = SlotTag;
	DragOp->GrabOffset = CachedGrabOffset;

	// 마우스를 따라다닐 비주얼: 같은 클래스의 위젯을 하나 더 만들고, 원래 슬롯 크기에 맞춰 SizeBox로 감싼다
	if (UA1EquipmentSlotWidget* DragVisual = CreateWidget<UA1EquipmentSlotWidget>(GetOwningPlayer(), GetClass()))
	{
		DragVisual->SetEquipmentInstance(EquipmentInstance);

		USizeBox* SizeWrapper = NewObject<USizeBox>(this);
		SizeWrapper->SetWidthOverride(CachedSlotSize.X);
		SizeWrapper->SetHeightOverride(CachedSlotSize.Y);
		SizeWrapper->AddChild(DragVisual);

		DragOp->DefaultDragVisual = SizeWrapper;
		// MouseDown 피벗 = 소스 위젯을 잡았던 지점이 드래그 비주얼에서도 그대로 마우스 아래에 오도록 엔진이 자동 계산
		DragOp->Pivot = EDragPivot::MouseDown;
	}

	// 원본 슬롯은 그대로 두고, 드래그 중임을 아이콘 반투명으로 표시
	SetDragVisualOpacity(true);

	OutOperation = DragOp;
}

void UA1EquipmentSlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	// 유효하지 않은 곳에 드롭되어 취소된 경우, 원본 아이콘의 반투명을 복구
	SetDragVisualOpacity(false);
}

void UA1EquipmentSlotWidget::SetDragVisualOpacity(bool bDragging)
{
	if (Image_Icon)
	{
		Image_Icon->SetRenderOpacity(bDragging ? 0.5f : 1.f);
	}
}

bool UA1EquipmentSlotWidget::CanAcceptItem(const UItemInstance* Item) const
{
	if (!Item || !SlotTag.IsValid())
	{
		return false;
	}

	const FItemFragment_Equipment* Fragment = Item->FindFragment<FItemFragment_Equipment>();
	if (!Fragment || !Fragment->EquipmentDefinition)
	{
		return false;
	}

	// 아이템의 장비 슬롯이 이 슬롯과 일치해야 장착 가능 (예: 헬멧은 헬멧 슬롯에만)
	return Fragment->EquipmentDefinition->SlotTag == SlotTag;
}

bool UA1EquipmentSlotWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	const UA1ItemDragDrop* DragOp = Cast<UA1ItemDragDrop>(InOperation);

	// 인벤토리에서 온, 이 슬롯에 맞는 아이템일 때만 드롭 대상으로 받아들인다
	if (DragOp && !DragOp->FromEquipmentSlotTag.IsValid() && CanAcceptItem(DragOp->ItemInstance))
	{
		return true;
	}

	return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
}

bool UA1EquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UA1ItemDragDrop* DragOp = Cast<UA1ItemDragDrop>(InOperation);
	if (!DragOp || !DragOp->ItemInstance)
	{
		return false;
	}

	// 장비창 → 장비창 이동은 아직 미지원 (인벤토리에서 온 드래그만 처리)
	if (DragOp->FromEquipmentSlotTag.IsValid())
	{
		return false;
	}

	// 슬롯에 맞지 않는 아이템 → Unhandled 반환으로 원본 위젯을 원상 복구시킨다
	if (!CanAcceptItem(DragOp->ItemInstance))
	{
		return false;
	}

	UInventoryComponent* Inventory = UInventoryComponent::FindInventoryComponent(GetOwningPlayer());
	if (!Inventory)
	{
		return false;
	}

	// 실제 장착은 서버 권위로 처리되고, 결과가 리플리케이션되면 양쪽 UI가 갱신된다
	//Inventory->EquipFromInventoryServer(DragOp->ItemId, SlotTag);
	return true;
}

void UA1EquipmentSlotWidget::SetEquipmentInstance(UEquipmentInstance* InEquipmentInstance)
{
	EquipmentInstance = InEquipmentInstance;

	const UItemInstance* SourceItem = EquipmentInstance ? EquipmentInstance->GetSourceItemInstance() : nullptr;

	if (Image_Icon)
	{
		if (SourceItem && SourceItem->Definition && SourceItem->Definition->Icon)
		{
			Image_Icon->SetBrushFromTexture(SourceItem->Definition->Icon, true);
			Image_Icon->SetVisibility(ESlateVisibility::HitTestInvisible);
			// 직전 드래그의 반투명이 남아있을 수 있으므로 복구
			Image_Icon->SetRenderOpacity(1.f);
		}
		else
		{
			Image_Icon->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	OnEquipmentChanged(EquipmentInstance);
}
