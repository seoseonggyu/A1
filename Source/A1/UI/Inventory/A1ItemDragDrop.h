// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/DragDropOperation.h"
#include "A1ItemDragDrop.generated.h"

class UItemInstance;
class UA1InventoryWidget;
class UA1InventoryItemWidget;

/**
 * 인벤토리 아이템 드래그&드롭 페이로드
 *
 * 드래그를 시작한 아이템 위젯/인벤토리와 잡은 위치 정보를 담아,
 * 드롭 시 목표 그리드 위치를 계산하는 데 사용합니다.
 */
UCLASS()
class A1_API UA1ItemDragDrop : public UDragDropOperation
{
	GENERATED_BODY()

public:
	/** 드래그 중인 아이템 ID */
	UPROPERTY()
	int32 ItemId = INDEX_NONE;

	/** 드래그 중인 아이템 인스턴스 */
	UPROPERTY()
	TObjectPtr<UItemInstance> ItemInstance = nullptr;

	/** 드래그를 시작한 인벤토리 위젯 */
	UPROPERTY()
	TObjectPtr<UA1InventoryWidget> FromInventory = nullptr;

	/** 드래그를 시작한 아이템 위젯 */
	UPROPERTY()
	TObjectPtr<UA1InventoryItemWidget> FromItemWidget = nullptr;

	/** 드래그 시작 시의 그리드 앵커 위치 */
	FIntPoint FromSlotPos = FIntPoint(-1, -1);

	/** 아이템 위젯 좌상단을 기준으로 마우스가 잡은 위치(px) */
	FVector2D GrabOffset = FVector2D::ZeroVector;
};
