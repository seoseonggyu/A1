// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widget/CommonExtensionUserWidget.h"
#include "A1InventoryItemWidget.generated.h"

class UWidget;
class UImage;
class UTextBlock;
class USizeBox;
class UItemInstance;
class UA1InventoryWidget;
class UA1InventoryItemTooltipWidget;
class UDragDropOperation;

DECLARE_LOG_CATEGORY_EXTERN(A1InventoryItemWidgetLog, Log, All);

/**
 * 인벤토리에 놓인 아이템 하나를 표시하는 위젯
 *
 * 아이콘·스택 수량을 그리고, SlotCount 크기만큼 셀을 차지하며,
 * 좌클릭 드래그로 이동을 시작합니다.
 */
UCLASS(Abstract)
class A1_API UA1InventoryItemWidget : public UCommonExtensionUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 아이템 위젯을 초기화합니다.
	 * @param InUnitCellSize 한 칸의 픽셀 크기
	 */
	void InitializeItem(UA1InventoryWidget* InOwnerInventory, UItemInstance* InItemInstance, int32 InStackCount, FVector2D InUnitCellSize);

	/** 스택 수량만 갱신합니다 */
	void RefreshStackCount(int32 InStackCount);
	
	int32 GetItemId() const;

	/** 드래그 중 반투명 등 처리 */
	void SetDragVisualOpacity(bool bDragging);

protected:
	//-----------------------------------------------------------------------------
	// UUserWidget 오버라이드 (호버 / 드래그)
	//-----------------------------------------------------------------------------

	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InPointerEvent, UDragDropOperation* InOperation) override;
	/** UMG 툴팁 시스템이 호버 시 호출. 툴팁 위젯을 생성해 아이템 정보를 채워 반환 */
	UFUNCTION()
	UWidget* HandleGetTooltipWidget();

	/** BP 비주얼 갱신 훅 (아이콘/이름/희귀도 등 추가 표현) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnItemRefreshed(UItemInstance* InItemInstance, int32 InStackCount);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UItemInstance> ItemInstance = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 StackCount = 0;

	UPROPERTY()
	TWeakObjectPtr<UA1InventoryWidget> OwnerInventory;

	/** 한 칸의 픽셀 크기 */
	FVector2D UnitCellSize = FVector2D::ZeroVector;

	/** 마우스 버튼을 누른 시점의 위젯 내 잡은 위치(px) */
	FVector2D CachedGrabOffset = FVector2D::ZeroVector;

	//-----------------------------------------------------------------------------
	// 설정 (BP에서 지정)
	//-----------------------------------------------------------------------------

	/** 호버 시 표시할 툴팁 위젯 클래스 (이름/설명) */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UA1InventoryItemTooltipWidget> TooltipWidgetClass;

	//-----------------------------------------------------------------------------
	// BindWidget (BP에서 배치)
	//-----------------------------------------------------------------------------

	/** 아이템 크기(SlotCount * UnitCellSize)를 강제하는 SizeBox */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USizeBox> SizeBox_Root;

	/** 아이템 아이콘 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_Icon;

	/** 스택 수량 텍스트 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Count;
};
