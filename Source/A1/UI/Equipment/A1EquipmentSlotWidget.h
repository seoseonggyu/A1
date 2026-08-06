// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widget/CommonExtensionUserWidget.h"
#include "GameplayTagContainer.h"
#include "A1EquipmentSlotWidget.generated.h"

class UImage;
class UEquipmentInstance;
class UItemInstance;
class UDragDropOperation;

DECLARE_LOG_CATEGORY_EXTERN(A1EquipmentSlotWidgetLog, Log, All);

/**
 * 장비창의 슬롯 하나(헬멧, 무기 등)를 표시하는 위젯
 *
 * 파페돌 레이아웃에서 이 위젯을 여러 번 배치하고, 인스턴스마다 SlotTag를 다르게
 * 지정해 자신이 어떤 장비 슬롯을 나타내는지 표시합니다. SetEquipmentInstance로
 * 장착된 아이템 아이콘을 갱신하고, 좌클릭 드래그로 이동을 시작합니다.
 */
UCLASS(Abstract)
class A1_API UA1EquipmentSlotWidget : public UCommonExtensionUserWidget
{
	GENERATED_BODY()

public:
	FGameplayTag GetSlotTag() const { return SlotTag; }

	/** 이 슬롯에 장착된 장비를 표시합니다 (nullptr이면 빈 슬롯) */
	void SetEquipmentInstance(UEquipmentInstance* InEquipmentInstance);

	UEquipmentInstance* GetEquipmentInstance() const { return EquipmentInstance; }

	/** 드래그 중 반투명 등 처리 */
	void SetDragVisualOpacity(bool bDragging);

protected:
	//-----------------------------------------------------------------------------
	// UUserWidget 오버라이드 (드래그)
	//-----------------------------------------------------------------------------

	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	/** 이 아이템을 이 슬롯에 장착할 수 있는지 (장비 프래그먼트가 있고 슬롯 태그가 일치) */
	bool CanAcceptItem(const UItemInstance* Item) const;

	/** BP 비주얼 갱신 훅 (아이콘 갱신 외 추가 연출이 필요할 때) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Equipment")
	void OnEquipmentChanged(UEquipmentInstance* InEquipmentInstance);

protected:
	//-----------------------------------------------------------------------------
	// 설정 (BP 인스턴스마다 지정)
	//-----------------------------------------------------------------------------

	/** 이 위젯이 나타내는 장비 슬롯 (파페돌에 배치한 인스턴스마다 다르게 지정) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment", meta = (Categories = "Equipment.Slot"))
	FGameplayTag SlotTag;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UEquipmentInstance> EquipmentInstance = nullptr;

	/** 마우스 버튼을 누른 시점의 위젯 내 잡은 위치(px) */
	FVector2D CachedGrabOffset = FVector2D::ZeroVector;

	/** 마우스 버튼을 누른 시점의 위젯 크기(px). 드래그 비주얼을 같은 크기로 만드는 데 사용 */
	FVector2D CachedSlotSize = FVector2D::ZeroVector;

	//-----------------------------------------------------------------------------
	// BindWidget (BP에서 배치)
	//-----------------------------------------------------------------------------

	/** 장착된 아이템 아이콘 (비어 있으면 Collapsed) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_Icon;
};
