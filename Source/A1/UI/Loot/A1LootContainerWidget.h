// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widget/CommonExtensionActivatableWidget.h"
#include "A1LootContainerWidget.generated.h"

class APawn;
class UA1InventoryWidget;

/**
 * UA1LootContainerWidget
 *
 * 월드 루팅 상자(AA1LootContainer)를 Interact했을 때 뜨는 창. UA1LootWidget과 동일한 구조지만
 * 상자는 장비 개념이 없으므로 UA1InventoryWidget만 대상(상자)에 바인딩한다. 내 인벤토리는
 * BP에 자유롭게 배치하면 되고 코드 개입이 필요 없다(UA1InventoryWidget이 기본적으로 소유 Pawn을
 * 바라보도록 되어 있음).
 *
 * InventoryComponent는 조건 없이 전체 복제되므로, 이 창은 상자 내용물을 실시간으로 반영한다 —
 * 같은 상자를 동시에 여는 다른 플레이어의 창도 함께 갱신된다. 이동은
 * UInventoryComponent::TransferInventoryToInventoryServer로 처리된다(UA1InventoryWidget의
 * NativeOnDrop에서 Source/Dest InventoryComponent를 비교해 분기).
 */
UCLASS(Abstract)
class A1_API UA1LootContainerWidget : public UCommonExtensionActivatableWidget
{
	GENERATED_BODY()

public:
	/** 대상(상자) 인벤토리 위젯을 대상 Pawn에 바인딩한다(양방향 드래그 가능). */
	void InitializeLoot(APawn* InTargetPawn);

protected:
	//-----------------------------------------------------------------------------
	// BindWidget (BP에서 배치) - 대상(상자) 쪽
	//-----------------------------------------------------------------------------

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UA1InventoryWidget> InventoryWidget_Other;
};
