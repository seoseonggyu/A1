// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widget/CommonExtensionUserWidget.h"
#include "A1EquipmentWidget.generated.h"

class APawn;
class UEquipmentComponent;
class UEquipmentInstance;
class UA1EquipmentSlotWidget;
struct FGameplayTag;

DECLARE_LOG_CATEGORY_EXTERN(A1EquipmentWidgetLog, Log, All);

/**
 * 장비창(파페돌 UI)
 *
 * 소유 Pawn의 UEquipmentComponent를 찾아, WidgetTree에 배치된 모든
 * UA1EquipmentSlotWidget 인스턴스를 각자의 SlotTag에 맞춰 갱신합니다.
 * 슬롯의 위치·모양은 BP에서 자유롭게 배치하고, 각 슬롯 인스턴스에 SlotTag만 지정하면 됩니다.
 *
 * 현재는 생성 시점 스냅샷만 반영합니다. 장착/해제가 실시간으로 반영되려면
 * EquipmentComponent에 변경 브로드캐스트 델리게이트가 추가되어야 하며, 인벤토리와의
 * 드래그 이동 연동과 함께 다음 단계에서 처리합니다.
 *
 * 클라이언트 전용으로 가정합니다.
 */
UCLASS(Abstract)
class A1_API UA1EquipmentWidget : public UCommonExtensionUserWidget
{
	GENERATED_BODY()

public:
	UEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent; }

	/**
	 * 소유 Pawn 대신 지정한 Pawn의 장비를 보여주도록 설정합니다 (예: 시체 루팅 창).
	 * bInReadOnly가 true면 모든 슬롯을 드래그 불가로 만듭니다(다른 액터의 장비라 이동 조작 대상이 아님).
	 * NativeConstruct의 SetupEquipment()보다 먼저 호출되었는지 여부와 무관하게 안전합니다
	 * (이미 다른 대상으로 구성되어 있었다면 다시 구성합니다).
	 */
	void SetTargetPawnOverride(APawn* InTargetPawn, bool bInReadOnly);

protected:
	//-----------------------------------------------------------------------------
	// UUserWidget 오버라이드
	//-----------------------------------------------------------------------------

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	/** EquipmentComponent를 찾고, WidgetTree에서 슬롯 위젯을 모두 수집해 갱신합니다 */
	void SetupEquipment();

	/** 현재 장착된 장비 상태를 모든 슬롯 위젯에 반영합니다 */
	void RefreshAllSlots();

	/** 특정 슬롯의 장비 변경 콜백 (장착/해제 시 해당 슬롯만 갱신) */
	void HandleEquipmentSlotChanged(FGameplayTag SlotTag, UEquipmentInstance* Instance);

	/** 위젯/참조를 정리합니다 */
	void TearDown();

private:
	/** 지정되어 있으면 GetOwningPlayerPawn() 대신 이 Pawn의 장비를 표시한다. */
	TWeakObjectPtr<APawn> TargetPawnOverride;

	/** true면 모든 슬롯 위젯을 읽기 전용(드래그 불가)으로 만든다. */
	bool bReadOnly = false;

	UPROPERTY()
	TObjectPtr<UEquipmentComponent> EquipmentComponent = nullptr;

	/** WidgetTree에서 수집한 슬롯 위젯들 (각자 SlotTag로 자신을 구분) */
	UPROPERTY()
	TArray<TObjectPtr<UA1EquipmentSlotWidget>> SlotWidgets;

	FDelegateHandle SlotChangedHandle;
};
