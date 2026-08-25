// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widget/CommonExtensionUserWidget.h"
#include "GameplayTagContainer.h"
#include "A1EquipmentSlotWidget.generated.h"

class UImage;
class UWidget;
class UTextBlock;
class UEquipmentInstance;
class UEquipmentComponent;
class UItemInstance;
class UDragDropOperation;
class UA1InventoryItemTooltipWidget;
class UA1InventoryCellWidget;

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

	/**
	 * 읽기 전용 여부를 설정합니다. true면 드래그로 옮기거나 받을 수 없습니다.
	 * 다른 액터(예: 시체)의 장비를 열람만 시켜줄 때 사용합니다(내 장비가 아니라 이동 조작 대상이 아님).
	 */
	void SetReadOnly(bool bInReadOnly) { bReadOnly = bInReadOnly; }

	/**
	 * 이 슬롯이 속한 EquipmentComponent를 등록합니다. 슬롯이 비어 있어도(EquipmentInstance가
	 * nullptr이어도) 어느 캐릭터의 장비창인지 알아야 캐릭터 간 드롭 대상을 판별할 수 있어 필요합니다.
	 */
	void SetOwnerEquipmentComponent(UEquipmentComponent* InOwnerEquipmentComponent) { OwnerEquipmentComponent = InOwnerEquipmentComponent; }

	UEquipmentComponent* GetOwnerEquipmentComponent() const;

protected:
	//-----------------------------------------------------------------------------
	// UUserWidget 오버라이드 (드래그)
	//-----------------------------------------------------------------------------

	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	/** UMG 툴팁 시스템이 호버 시 호출. 툴팁 위젯을 생성해 장착된 아이템 정보(이름/설명)를 채워 반환 */
	UFUNCTION()
	UWidget* HandleGetTooltipWidget();

	/** 이 아이템을 이 슬롯에 장착할 수 있는지 (장비 프래그먼트가 있고 슬롯 태그가 일치) */
	bool CanAcceptItem(const UItemInstance* Item) const;

	/**
	 * CanAcceptItem에 더해, 이미 장착된 아이템이 있을 경우 그 아이템을 되돌릴 인벤토리의 빈 칸이
	 * 있는지까지 확인합니다(원자적 교체 규칙의 미리보기 버전). 드래그 오버 미리보기와 드롭 실행이
	 * 항상 같은 기준으로 판단하도록 두 곳 모두 이 함수를 통해서만 수락 여부를 결정합니다.
	 */
	bool CanAcceptItemFull(const UItemInstance* Item) const;

	/** 드래그 오버 중인 아이템의 배치 가능 여부에 따라 Cell_Highlight를 초록/빨강으로 표시합니다 (Inventory와 동일한 위젯 재사용) */
	void UpdateDropHighlight(bool bCanPlace);

	/** 하이라이트를 기본 상태로 되돌립니다 */
	void ClearDropHighlight();


protected:
	//-----------------------------------------------------------------------------
	// 설정 (BP 인스턴스마다 지정)
	//-----------------------------------------------------------------------------

	/** 이 위젯이 나타내는 장비 슬롯 (파페돌에 배치한 인스턴스마다 다르게 지정) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment", meta = (Categories = "Equipment.Slot"))
	FGameplayTag SlotTag;

	/** 호버 시 표시할 툴팁 위젯 클래스 (이름/설명). Inventory와 동일한 클래스를 공유합니다 */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment")
	TSubclassOf<UA1InventoryItemTooltipWidget> TooltipWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UEquipmentInstance> EquipmentInstance = nullptr;

	/** true면 드래그로 옮기거나 받을 수 없다. */
	bool bReadOnly = false;

	/** 이 슬롯이 속한 EquipmentComponent (슬롯이 비어 있어도 유효, SetOwnerEquipmentComponent로 등록됨) */
	TWeakObjectPtr<UEquipmentComponent> OwnerEquipmentComponent;

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

	/** 장착된 아이템 수량 (1개 이하면 빈 텍스트) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Count;

	/** 드래그 오버 시 배치 가능 여부를 초록/빨강으로 표시하는 하이라이트 (Inventory 그리드 셀과 동일한 위젯) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UA1InventoryCellWidget> Cell_Highlight;
};
