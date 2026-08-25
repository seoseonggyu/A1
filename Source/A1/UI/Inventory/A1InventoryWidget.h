// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widget/CommonExtensionActivatableWidget.h"
#include "A1InventoryWidget.generated.h"

class APawn;
class UUniformGridPanel;
class UCanvasPanel;
class UInventoryComponent;
class UItemInstance;
class UItemDefinition;
class UA1InventoryCellWidget;
class UA1InventoryItemWidget;
class UA1ItemDragDrop;
class UDragDropOperation;
enum class EInventoryGridChangeType : uint8;

DECLARE_LOG_CATEGORY_EXTERN(A1InventoryWidgetLog, Log, All);

/** 인벤토리 창이 닫힐 때(비활성화) 브로드캐스트됩니다. Ability 등에서 EndAbility 트리거용으로 바인딩합니다 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnA1InventoryWindowClosed);

/**
 * 인벤토리 창 (활성화 가능 위젯)
 *
 * 소유 Pawn(또는 SetTargetPawnOverride로 지정한 다른 Pawn)의 UInventoryComponent를 찾아
 * 2D 그리드를 구성하고, OnInventoryGridChanged를 구독해 아이템 위젯을 생성/이동/제거합니다.
 * UInventoryComponent는 EquipmentComponent와 마찬가지로 조건 없이 전체 복제되므로,
 * 다른 플레이어(예: 사망한 캐릭터)의 인벤토리를 지정해도 실시간으로 갱신된다 — 같은 컴포넌트를
 * 바라보는 다른 클라이언트가 있다면 그쪽 위젯도 동시에 갱신된다.
 * 드래그&드롭으로 아이템을 옮기며, 배치 가능 여부를 초록/빨강으로 미리보기합니다.
 *
 * 클라이언트 전용으로 가정합니다.
 */
UCLASS(Abstract)
class A1_API UA1InventoryWidget : public UCommonExtensionActivatableWidget
{
	GENERATED_BODY()

public:
	UA1InventoryWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }
	FVector2D GetUnitCellSize() const { return UnitCellSize; }

	/**
	 * 소유 Pawn 대신 지정한 Pawn의 인벤토리를 보여주도록 설정합니다 (예: 시체 루팅 창).
	 * bInReadOnly가 true면 드래그 자체를 막습니다. false면 캐릭터 간 드래그&드롭도 가능합니다
	 * (NativeOnDrop에서 Source/Dest InventoryComponent를 비교해 같은 캐릭터인지 판별).
	 * NativeConstruct의 SetupInventory()보다 먼저 호출되었는지 여부와 무관하게 안전합니다
	 * (이미 다른 대상으로 구성되어 있었다면 다시 구성합니다).
	 */
	void SetTargetPawnOverride(APawn* InTargetPawn, bool bInReadOnly);

protected:
	//-----------------------------------------------------------------------------
	// UCommonActivatableWidget / UUserWidget 오버라이드
	//-----------------------------------------------------------------------------

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	//-----------------------------------------------------------------------------
	// 드래그 & 드롭
	//-----------------------------------------------------------------------------

	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

private:
	//-----------------------------------------------------------------------------
	// 그리드 구성
	//-----------------------------------------------------------------------------

	/** InventoryComponent를 찾아 캐싱하고 그리드를 구성합니다 */
	void SetupInventory();

	/** 배경 셀 위젯을 그리드 크기만큼 생성합니다 */
	void BuildCells();

	/** 현재 인벤토리의 모든 아이템 위젯을 생성합니다 */
	void SpawnExistingItems();

	/** 위젯/구독을 정리합니다 */
	void TearDown();

	//-----------------------------------------------------------------------------
	// 델리게이트 / 드래그 헬퍼
	//-----------------------------------------------------------------------------

	/** 인벤토리 그리드 변경 콜백 */
	void HandleGridChanged(EInventoryGridChangeType ChangeType, int32 ItemId, UItemInstance* Instance, const FIntPoint& SlotPos, int32 StackCount);

	/** ItemId에 해당하는 아이템 위젯을 (없으면 생성해) 지정 위치에 배치합니다 */
	void PlaceItemWidget(int32 ItemId, UItemInstance* Instance, const FIntPoint& SlotPos, int32 StackCount);

	/** ItemId의 아이템 위젯을 제거합니다 */
	void RemoveItemWidget(int32 ItemId);

	/** (x, y) → 셀 배열 인덱스 */
	int32 CellIndex(int32 X, int32 Y) const;

	/** 드래그 이벤트의 스크린 좌표 + GrabOffset으로 아이템이 놓일 그리드 앵커 좌표를 계산합니다 */
	FIntPoint ComputeDropAnchor(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, const FVector2D& GrabOffset) const;

	/** Anchor~Anchor+Size 범위의 셀 하이라이트를 초록/빨강으로 갱신합니다 */
	void UpdateDropPreview(const FIntPoint& Anchor, const FIntPoint& Size, bool bCanPlace);

	/** 직전에 하이라이트한 셀들을 기본 상태로 되돌립니다 */
	void ClearDropPreview();

protected:
	//-----------------------------------------------------------------------------
	// 설정 (BP에서 지정)
	//-----------------------------------------------------------------------------

	/** 한 칸의 픽셀 크기 */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	FVector2D UnitCellSize = FVector2D(64.f, 64.f);

	/** 배경 셀 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UA1InventoryCellWidget> CellWidgetClass;

	/** 아이템 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UA1InventoryItemWidget> ItemWidgetClass;

	//-----------------------------------------------------------------------------
	// BindWidget (BP에서 배치)
	//-----------------------------------------------------------------------------

	/** 배경 셀을 담는 UniformGrid */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> GridPanel_Cells;

	/** 아이템 위젯을 픽셀 위치로 배치하는 Canvas */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel_Items;

private:
	/** 지정되어 있으면 GetOwningPlayer() 대신 이 Pawn의 인벤토리를 표시한다. */
	TWeakObjectPtr<APawn> TargetPawnOverride;

	/** true면 드래그로 옮길 수 없다(모든 아이템 위젯에 전파). */
	bool bReadOnly = false;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> InventoryComponent = nullptr;

	/** 배경 셀 위젯 (인덱스 = y * GridSize.X + x) */
	UPROPERTY()
	TArray<TObjectPtr<UA1InventoryCellWidget>> CellWidgets;

	/** ItemId → 아이템 위젯 */
	UPROPERTY()
	TMap<int32, TObjectPtr<UA1InventoryItemWidget>> ItemWidgets;

	/** 드래그 미리보기용으로 CanvasPanel_Items에 추가한 하이라이트 셀 위젯 (아이템 아이콘보다 위에 그려짐) */
	UPROPERTY()
	TArray<TObjectPtr<UA1InventoryCellWidget>> DropPreviewCellWidgets;

	/** 캐싱된 그리드 크기 */
	FIntPoint CachedGridSize = FIntPoint::ZeroValue;

	/** 직전 미리보기 앵커 (중복 계산 방지) */
	FIntPoint PrevPreviewAnchor = FIntPoint(-999, -999);

	FDelegateHandle GridChangedHandle;
};
