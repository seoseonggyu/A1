// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Equipment/A1EquipmentWidget.h"
#include "UI/Equipment/A1EquipmentSlotWidget.h"

#include "Equipment/EquipmentComponent.h"

#include "Blueprint/WidgetTree.h"
#include "GameFramework/Pawn.h"

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

	APawn* SourcePawn = TargetPawnOverride.IsValid() ? TargetPawnOverride.Get() : GetOwningPlayerPawn();

	EquipmentComponent = UEquipmentComponent::FindEquipmentComponent(SourcePawn);
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
				SlotWidget->SetReadOnly(bReadOnly);
				SlotWidget->SetOwnerEquipmentComponent(EquipmentComponent);
				SlotWidgets.Add(SlotWidget);
			}
		});
	}

	RefreshAllSlots();

	// 읽기 전용이어도(예: 시체) 다른 플레이어의 조작으로 상태가 바뀔 수 있으므로 항상 구독한다.
	SlotChangedHandle = EquipmentComponent->OnEquipmentSlotChanged.AddUObject(this, &ThisClass::HandleEquipmentSlotChanged);
}

void UA1EquipmentWidget::SetTargetPawnOverride(APawn* InTargetPawn, bool bInReadOnly)
{
	TargetPawnOverride = InTargetPawn;
	bReadOnly = bInReadOnly;

	// SetupEquipment()가 이 호출보다 먼저 실행되어 (초기화 순서는 보장되지 않으므로) 잘못된
	// 대상(예: 내 소유 Pawn)으로 이미 구성되어 있었을 수 있으므로 정리 후 다시 구성한다.
	TearDown();
	SetupEquipment();
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
