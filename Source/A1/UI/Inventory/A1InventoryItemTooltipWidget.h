// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widget/CommonExtensionUserWidget.h"
#include "A1InventoryItemTooltipWidget.generated.h"

class UTextBlock;
class UItemInstance;

DECLARE_LOG_CATEGORY_EXTERN(A1InventoryItemTooltipWidgetLog, Log, All);

/**
 * 아이템 위에 마우스를 올렸을 때 표시되는 툴팁 위젯
 *
 * ItemDefinition의 DisplayName / Description을 사각형 안에 표시합니다.
 * 시각 요소는 BP에서 디자인하고, 내용 채우기는 SetItem으로 제어합니다.
 */
UCLASS(Abstract)
class A1_API UA1InventoryItemTooltipWidget : public UCommonExtensionUserWidget
{
	GENERATED_BODY()

public:
	/** ItemDefinition의 DisplayName / Description으로 툴팁 내용을 채웁니다 */
	void SetItem(UItemInstance* InItemInstance);

protected:
	//-----------------------------------------------------------------------------
	// BindWidget (BP에서 배치)
	//-----------------------------------------------------------------------------

	/** 아이템 표시 이름 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Name;

	/** 아이템 설명 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Description;
};
