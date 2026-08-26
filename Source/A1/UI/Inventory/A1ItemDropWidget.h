// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widget/CommonExtensionUserWidget.h"
#include "A1ItemDropWidget.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1ItemDropWidgetLog, Log, All);

/**
 * 인벤토리/장비창이 아닌 제3의 드롭 존 위젯 (예: 버리기 UI)
 *
 * UA1ItemDragDrop 페이로드의 ItemInstance를 UA1Ability_DropItem에 GameplayEvent로 실어
 * 보낸다. 장착 여부·표시 메시 판별 등 실제 드롭 처리는 어빌리티(서버 권위)가 담당한다.
 */
UCLASS(Abstract)
class A1_API UA1ItemDropWidget : public UCommonExtensionUserWidget
{
	GENERATED_BODY()

protected:
	//-----------------------------------------------------------------------------
	// UUserWidget 오버라이드 (드래그 & 드롭)
	//-----------------------------------------------------------------------------

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};
