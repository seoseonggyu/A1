// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Ability/CommonGameplayAbility.h"
#include "A1Ability_Interact_Player.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1AbilityInteractPlayerLog, Log, All);

class UA1LootWidget;

/**
 * UA1Ability_Interact_Player
 *
 * 사망한 다른 플레이어(시체)를 대상으로 한 상호작용 어빌리티. UA1Ability_Interact가 대상의
 * InteractEventTag(GameplayEvent.Interact.Player)로 서버에서만 트리거한다
 * (TriggerEventData->Target이 시체가 된 AA1Character).
 *
 * 루팅 창(UA1LootWidget)은 새 로직 없이 플레이어 본인 화면에도 쓰이는 UA1EquipmentWidget/
 * UA1InventoryWidget을 그대로 재사용한다. EquipmentComponent/InventoryComponent 모두 조건 없이
 * 전체 복제되므로(스냅샷 불필요) 소유 클라가 직접 대상 Pawn에 라이브로 바인딩할 수 있다 —
 * 같은 시체를 동시에 열람 중인 다른 플레이어의 창도 실시간으로 함께 갱신된다.
 *
 * 실행 위치:
 *  - NetExecutionPolicy = ServerInitiated. 서버가 로컬로 발동을 결정(HandleGameplayEvent)하면
 *    그 활성화가 소유 클라에도 복제되어, 서버·소유 클라가 각자 자기 인스턴스에서 ActivateAbility를
 *    실행한다. TriggerEventData->Target은 양쪽 모두에서 사용 가능하므로 별도 RPC 없이
 *    HasAuthority 분기만으로 "소유 클라에서만 위젯을 띄운다"를 구현한다.
 */
UCLASS()
class A1_API UA1Ability_Interact_Player : public UCommonGameplayAbility
{
	GENERATED_BODY()

public:
	UA1Ability_Interact_Player(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	/** 루팅 창을 띄우고 대상(시체)에 바인딩한다. 소유 클라 전용(비복제). */
	void ShowLootWidgetLocal(AActor* TargetActor);

protected:
	/**
	 * 루팅 창 위젯 클래스. UA1EquipmentWidget/UA1InventoryWidget을 자식으로 배치한 BP를 지정한다
	 * (레이아웃만 새로 구성하고, 로직은 두 기존 위젯을 그대로 재사용).
	 */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Interact")
	TSubclassOf<UA1LootWidget> LootWidgetClass;

	/** 루팅 창을 띄울 UI 레이어 */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Interact", meta = (Categories = "UI.Layer"))
	FGameplayTag WidgetLayerTag;
};
