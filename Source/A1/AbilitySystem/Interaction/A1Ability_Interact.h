#pragma once

#include "AbilitySystem/Ability/CommonGameplayAbility.h"
#include "GameplayTagContainer.h"
#include "A1Ability_Interact.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1AbilityInteractLog, Log, All);

/**
 * UA1Ability_Interact
 *
 * 상호작용 입력(Input.Ability.Interact)이 들어온 순간, 커서가 올라가 있던 Interactable을
 * 대상으로 스냅샷하고, 플레이어가 (WASD로) 그 대상의 InteractionRange 이내로 접근하면
 * 서버에서 상호작용을 실행하는 어빌리티.
 *
 * 자동 이동(따라가기)은 하지 않는다 — 접근은 사용자가 WASD로 한다.
 * 대상과의 거리는 소유 클라에서 매 프레임(WaitForTick) 감시하며, 근접 시:
 *  - 호스트/스탠드얼론(Authority) : 즉시 서버 실행.
 *  - 소유 클라(원격)             : CommitInteractServer RPC로 서버에 실행 요청.
 * 서버는 거리·CanInteract를 재검증한 뒤 IA1Interactable::OnInteractAuth를 호출한다.
 *
 * 서버 RPC 사용을 위해 ReplicationPolicy=ReplicateYes. 대상/거리 정보는 서버가
 * 대상 액터로부터 다시 수집하므로 별도 복제가 필요 없다.
 */
UCLASS()
class A1_API UA1Ability_Interact : public UCommonGameplayAbility
{
	GENERATED_BODY()

public:
	UA1Ability_Interact(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	/** 매 프레임 아바타-대상 거리를 검사한다. 소유 클라에서만 돈다. */
	UFUNCTION()
	void OnTick(float DeltaTime);

	/** 근접 판정이 성립했을 때 서버 실행 경로로 분기한다. (Authority면 직접, 아니면 RPC) */
	void CommitInteractLocal();

	/** 소유 클라 → 서버. 서버에서 거리·조건을 재검증하고 상호작용을 실행한다. */
	UFUNCTION(Server, Reliable)
	void CommitInteractServer(AActor* Target);

	/** 서버 전용. 대상의 조건/거리를 재검증한 뒤 OnInteractAuth를 실행한다. */
	void PerformInteractionAuth(AActor* Target);

	/** 대상으로부터 InteractionRange를 다시 수집한다. (없으면 기본값) */
	float GetInteractionRangeFor(AActor* Target) const;

private:
	/** 활성화 시점에 스냅샷한 상호작용 대상. */
	TWeakObjectPtr<AActor> TargetActor;

	/** 대상 근접 발동 거리(cm). 활성화 시점 대상 옵션에서 캐시. */
	float InteractionRange = 150.f;

	/** 서버 거리 재검증 시 허용 오차(cm). 예측/지연 보정. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Interact")
	float ServerRangeTolerance = 100.f;
};
