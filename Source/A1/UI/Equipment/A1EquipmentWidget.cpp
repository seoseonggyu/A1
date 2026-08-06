// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Equipment/A1EquipmentWidget.h"
#include "UI/Equipment/A1EquipmentSlotWidget.h"

#include "Equipment/EquipmentComponent.h"

#include "Blueprint/WidgetTree.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1EquipmentWidget)

DEFINE_LOG_CATEGORY(A1EquipmentWidgetLog);

void UA1EquipmentWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetupEquipment();
}

void UA1EquipmentWidget::NativeDestruct()
{
	TearDown();

	Super::NativeDestruct();
}

void UA1EquipmentWidget::SetupEquipment()
{
	if (EquipmentComponent)
	{
		// 이미 구성됨 (재활성화 등)
		return;
	}

	EquipmentComponent = UEquipmentComponent::FindEquipmentComponent(GetOwningPlayerPawn());
	if (!EquipmentComponent)
	{
		UE_LOG(A1EquipmentWidgetLog, Warning, TEXT("SetupEquipment: EquipmentComponent를 찾을 수 없습니다"));
		return;
	}

	SlotWidgets.Reset();
	if (WidgetTree)
	{
		WidgetTree->ForEachWidget([this](UWidget* InWidget)
		{
			if (UA1EquipmentSlotWidget* SlotWidget = Cast<UA1EquipmentSlotWidget>(InWidget))
			{
				SlotWidgets.Add(SlotWidget);
			}
		});
	}

	RefreshAllSlots();

	// 장착/해제가 실시간으로 반영되도록 구독
	SlotChangedHandle = EquipmentComponent->OnEquipmentSlotChanged.AddUObject(this, &ThisClass::HandleEquipmentSlotChanged);
}

void UA1EquipmentWidget::HandleEquipmentSlotChanged(FGameplayTag SlotTag, UEquipmentInstance* Instance)
{
	for (UA1EquipmentSlotWidget* SlotWidget : SlotWidgets)
	{
		if (SlotWidget && SlotWidget->GetSlotTag() == SlotTag)
		{
			SlotWidget->SetEquipmentInstance(Instance);
		}
	}
}

void UA1EquipmentWidget::RefreshAllSlots()
{
	if (!EquipmentComponent)
	{
		return;
	}

	for (UA1EquipmentSlotWidget* SlotWidget : SlotWidgets)
	{
		if (!SlotWidget || !SlotWidget->GetSlotTag().IsValid())
		{
			continue;
		}

		SlotWidget->SetEquipmentInstance(EquipmentComponent->GetEquipmentInSlot(SlotWidget->GetSlotTag()));
	}
}

void UA1EquipmentWidget::TearDown()
{
	if (EquipmentComponent)
	{
		EquipmentComponent->OnEquipmentSlotChanged.Remove(SlotChangedHandle);
	}
	SlotChangedHandle.Reset();

	SlotWidgets.Reset();
	EquipmentComponent = nullptr;
}
