// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widget/CommonExtensionActivatableWidget.h"
#include "A1LootWidget.generated.h"

class APawn;
class UA1EquipmentWidget;
class UA1InventoryWidget;

/**
 * UA1LootWidget
 *
 * 사망한 다른 플레이어를 Interact했을 때 뜨는 창. 왼쪽(내 장비/인벤토리)은 BP에 자유롭게 배치하면
 * 되고 코드 개입이 필요 없다(UA1EquipmentWidget/UA1InventoryWidget 둘 다 기본적으로 소유 Pawn을
 * 바라보도록 되어 있음). 오른쪽(대상) 위젯만 InitializeLoot에서 대상 시체로 다시 바인딩한다.
 *
 * EquipmentComponent/InventoryComponent 모두 조건 없이 전체 복제되므로, 이 창은 대상의 데이터를
 * 실시간으로 반영한다 — 같은 시체를 동시에 열람 중인 다른 플레이어의 창도 함께 갱신된다.
 * 양쪽 다 드래그로 실제 이동이 가능하다(내 것 ↔ 대상 것, 양방향). 캐릭터 간 이동은
 * UInventoryComponent::Transfer*Server 계열 RPC로 처리되며, 같은 캐릭터 내 이동/장착은 기존
 * MoveItemServer/EquipFromInventoryServer/UnequipToInventoryServer가 그대로 담당한다
 * (UA1InventoryWidget/UA1EquipmentSlotWidget의 NativeOnDrop에서 Source/Dest 컴포넌트를 비교해 분기).
 *
 * 새 로직을 추가하지 않고 기존 UA1EquipmentWidget/UA1InventoryWidget을 그대로 재사용하기 위한
 * 최소한의 레이아웃 컨테이너다. (같은 위젯 타입이 좌/우 두 개라 FindWidgetOfType만으로는
 * 어느 쪽이 "대상" 쪽인지 구분할 수 없어 이름 있는 BindWidget으로 명시한다)
 */
UCLASS(Abstract)
class A1_API UA1LootWidget : public UCommonExtensionActivatableWidget
{
	GENERATED_BODY()

public:
	/** 오른쪽(대상) 장비/인벤토리 위젯을 대상 Pawn에 바인딩한다(양방향 드래그 가능). */
	void InitializeLoot(APawn* InTargetPawn);

protected:
	//-----------------------------------------------------------------------------
	// BindWidget (BP에서 배치) - 대상(시체) 쪽
	//-----------------------------------------------------------------------------

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UA1EquipmentWidget> EquipmentWidget_Other;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UA1InventoryWidget> InventoryWidget_Other;
};
