// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widget/CommonExtensionUserWidget.h"
#include "A1InventoryCellWidget.generated.h"

class UImage;

DECLARE_LOG_CATEGORY_EXTERN(A1InventoryCellWidgetLog, Log, All);

/** 인벤토리 그리드 한 칸의 상태 */
UENUM(BlueprintType)
enum class EA1InventorySlotState : uint8
{
	Default,
	Valid,
	Invalid
};

/**
 * 인벤토리 그리드의 배경 셀 위젯
 *
 * 각 칸의 배경을 그리며, 드래그 중 배치 가능 여부를 초록/빨강으로 표시합니다.
 */
UCLASS(Abstract)
class A1_API UA1InventoryCellWidget : public UCommonExtensionUserWidget
{
	GENERATED_BODY()

public:
	/** 셀 상태(기본/초록/빨강)를 변경합니다 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetSlotState(EA1InventorySlotState NewState);

protected:
	virtual void NativeOnInitialized() override;

protected:
	/** 초록/빨강 하이라이트용 이미지 (선택). 없으면 BP OnSlotStateChanged로 처리 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_Valid;

	/** 배치 가능 색상 */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	FLinearColor ValidColor = FLinearColor(0.f, 1.f, 0.f, 0.35f);

	/** 배치 불가 색상 */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	FLinearColor InvalidColor = FLinearColor(1.f, 0.f, 0.f, 0.35f);

private:
	EA1InventorySlotState CurrentState = EA1InventorySlotState::Default;
};
