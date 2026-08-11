// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Ability/CommonGameplayAbility.h"
#include "A1Ability_Interact.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1AbilityInteractLog, Log, All);

/**
 * UA1Ability_Interact
 *
 * 실제 상호작용을 "실행"하는 어빌리티. 입력으로 직접 발동하지 않고,
 * 주변 스캔 어빌리티(UA1Ability_Interact_Scan)가 대상을 찾은 상태에서 상호작용 입력이
 * 들어오면 보내는 GameplayEvent(GameplayEvent.Interact)로 트리거된다.
 * TriggerEventData->Target 이 상호작용 대상 액터다.
 *
 * 실행 위치:
 *  - NetExecutionPolicy = LocalPredicted(베이스 기본값). 소유 클라에서 트리거되면 클라 예측과 함께
 *    서버로 활성화 RPC(+TriggerEventData)가 전달된다.
 *  - 결과 처리(IA1Interactable::OnInteractAuth)는 서버(Authority)에서만 수행한다.
 *  - 대상이 확장 이벤트 태그(FA1InteractionOption::InteractEventTag)를 제공하면 서버에서
 *    그 GameplayEvent를 소유자 ASC로 보내 후속 어빌리티(줍기/문 열기 등)를 실행할 수 있게 한다.
 */
UCLASS()
class A1_API UA1Ability_Interact : public UCommonGameplayAbility
{
	GENERATED_BODY()

public:
	UA1Ability_Interact(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
