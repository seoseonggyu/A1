// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Ability/CommonGameplayAbility.h"
#include "Coro.h"
#include "A1Ability_Interact_Door.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1AbilityInteractDoorLog, Log, All);

class UAnimMontage;

/**
 * UA1Ability_Interact_Door
 *
 * 문 전용 상호작용 어빌리티. UA1Ability_Interact가 대상의 InteractEventTag(GameplayEvent.Interact.Door)로
 * 서버에서만 트리거한다(TriggerEventData->Target이 문 액터).
 *
 * UA1Ability_Sprint_Check → UA1Ability_Sprint_Active와 동일한 "Check가 Active를 GameplayEvent로 깨우고,
 * Active가 WaitInputRelease로 입력 유지 여부를 스스로 감시" 패턴을 재사용한다.
 * HoldDuration(기본 2초) 동안 상호작용 입력을 유지하면 문을 연다. 도중에 입력을 떼면 취소된다.
 *
 * 입력 유지 감지 전제조건 (에셋 설정 필요):
 *  - 이 어빌리티가 UA1Ability_Interact_Scan과 "같은" InputTag(Input.Ability.Interact)로 부여되어야
 *    UAbilityTask_WaitInputRelease가 릴리즈 신호를 받을 수 있다. (CommonAbilitySystemComponent가
 *    DynamicSpecSourceTags에 해당 InputTag를 가진 "모든" 활성 스펙에 릴리즈를 통지하는 방식이기 때문)
 *
 * 실행 위치:
 *  - NetExecutionPolicy = ServerInitiated. 서버가 로컬로 발동을 결정(HandleGameplayEvent)하면
 *    그 활성화가 소유 클라에도 복제되어, 서버·소유 클라가 각자 자기 쪽에서 홀드/몽타주를 진행한다.
 *  - 결과 처리(IA1Interactable::OnInteractAuth)는 서버 인스턴스가 홀드를 완주했을 때만 수행한다.
 */
UCLASS()
class A1_API UA1Ability_Interact_Door : public UCommonGameplayAbility
{
	GENERATED_BODY()

public:
	UA1Ability_Interact_Door(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	/** HoldDuration만큼 대기한 뒤, 그동안 입력이 유지됐으면 문 결과 처리를 수행한다. */
	TCoroTask<void> RunHoldCoroutine();

	/** 상호작용 입력을 떼면 호출된다. 아직 홀드가 끝나지 않았다면 취소한다. */
	UFUNCTION()
	void OnInputReleased(float TimeHeld);

protected:
	/** 문을 여는 데 필요한 입력 유지 시간(초). */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Interact")
	float HoldDuration = 2.f;

	/** 홀드 중 재생할 모션(선택). 미지정이면 몽타주 없이 홀드만 판정한다. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Interact")
	TObjectPtr<UAnimMontage> HoldMontage;

private:
	/** 홀드 대상 문 액터. */
	TWeakObjectPtr<AActor> TargetActor;

	/** 홀드 완료 전에 입력이 풀렸는지 여부. */
	bool bInputReleased = false;

	/** RunHoldCoroutine의 소유권. 어빌리티 인스턴스가 살아있는 동안 유지되어야 한다. */
	TCoroTask<void> PendingHoldTask;
};
