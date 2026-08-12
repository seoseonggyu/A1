// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Ability/CommonGameplayAbility.h"
#include "A1Ability_Interact_Pickup.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1AbilityInteractPickupLog, Log, All);

class UAnimMontage;

/**
 * UA1Ability_Interact_Pickup
 *
 * 줍기 전용 상호작용 어빌리티. UA1Ability_Interact가 대상의 InteractEventTag(GameplayEvent.Interact.Pickup)로
 * 서버에서만 트리거한다(TriggerEventData->Target이 대상 액터).
 *
 * 문/레버와 달리 별도의 대기·판정이 필요 없는 단순 동작이므로, 결과 처리(OnInteractAuth)는
 * 몽타주 재생과 무관하게 즉시 수행한다. PickupMontage는 순수 연출이며 재생이 끝나면(정상/중단 무관)
 * 어빌리티를 종료한다.
 *
 * 실행 위치:
 *  - NetExecutionPolicy = ServerInitiated. 서버가 로컬로 발동을 결정(HandleGameplayEvent)하면
 *    그 활성화가 소유 클라에도 복제되어, 서버·소유 클라가 각자 자기 쪽에서 몽타주를 재생한다.
 *    (원격 클라는 어빌리티 인스턴스 없이 ASC의 몽타주 복제로만 연출을 본다)
 *  - 결과 처리(IA1Interactable::OnInteractAuth)는 서버 인스턴스에서만 수행한다.
 */
UCLASS()
class A1_API UA1Ability_Interact_Pickup : public UCommonGameplayAbility
{
	GENERATED_BODY()

public:
	UA1Ability_Interact_Pickup(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	/** 몽타주 재생이 끝나면(정상 종료/중단/취소 모두) 어빌리티를 종료한다. */
	UFUNCTION()
	void OnMontageEnded();

protected:
	/** 줍기 모션. 비어있으면 몽타주 없이 결과만 즉시 처리하고 종료한다. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Interact")
	TObjectPtr<UAnimMontage> PickupMontage;
};
