// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Interaction/A1Ability_Interact_Hold.h"
#include "A1Ability_Interact_LootContainer.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1AbilityInteractLootContainerLog, Log, All);

class UA1LootContainerWidget;

/**
 * UA1Ability_Interact_LootContainer
 *
 * 월드 루팅 상자(AA1LootContainer) 전용 상호작용 어빌리티. UA1Ability_Interact가 대상의
 * InteractEventTag(GameplayEvent.Interact.LootContainer)로 서버에서만 트리거한다
 * (TriggerEventData->Target이 AA1LootContainer).
 *
 * 홀드 판정 자체는 UA1Ability_Interact_Hold가 담당하고, 이 클래스는 홀드를 완주했을 때
 * 소유 클라 인스턴스에서 루팅 창(UA1LootContainerWidget)을 여는 로컬 연출만 담당한다.
 * 즉시 지급이 아니라 창을 열어 드래그로 하나씩 가져가는 방식이다.
 */
UCLASS()
class A1_API UA1Ability_Interact_LootContainer : public UA1Ability_Interact_Hold
{
	GENERATED_BODY()

public:
	UA1Ability_Interact_LootContainer(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	/** 홀드를 완주한 소유 클라 인스턴스에서만 호출된다. 루팅 창을 띄우고 대상(상자)에 바인딩한다. */
	virtual void OnHoldCompletedLocal(AActor* Interactor, AActor* Target) override;

protected:
	/** 루팅 창 위젯 클래스. UA1InventoryWidget을 자식으로 배치한 BP를 지정한다. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Interact")
	TSubclassOf<UA1LootContainerWidget> LootWidgetClass;

	/** 루팅 창을 띄울 UI 레이어 */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Interact", meta = (Categories = "UI.Layer"))
	FGameplayTag LootWidgetLayerTag;
};
