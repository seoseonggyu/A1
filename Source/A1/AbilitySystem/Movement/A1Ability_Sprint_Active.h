#pragma once

#include "AbilitySystem/Ability/CommonGameplayAbility.h"
#include "A1Ability_Sprint_Active.generated.h"

class UGameplayEffect;

/**
 * UA1Ability_Sprint_Active
 *
 * 실제 스프린트를 지속 처리하는 어빌리티. UA1Ability_Sprint_Check가 발송한
 * GameplayEvent(Ability.Sprint.Active)로만 트리거되며(Manual + GameplayEvent), Status.Sprint 태그를 부여한다.
 *
 * 지속 동안:
 *  - WaitForTick으로 매 프레임 정지/낙하 여부를 감시하고, 멈추면 종료한다.
 *  - WaitInputRelease로 입력을 떼면 종료한다.
 *  - CommitInterval 주기 타이머로 스태미나(Cost)를 소모하며, 소모 실패 시 종료한다.
 *
 * 이동속도는 A1에 속도 Attribute가 없으므로 GameplayEffect가 아니라 CharacterMovement의
 * MaxWalkSpeed를 로컬(서버·소유 클라 각각)에서 배율로 올렸다가 종료 시 원복하는 방식으로 처리한다.
 * (SetMoveInputBlockedLocal / SetOrientRotationToMovementLocal과 동일한 비복제 로컬 패턴)
 *
 * 종료 시 이동속도를 원복하고 회복 억제 효과를 부여한다.
 * 서버 권위 종료를 위해 bServerRespectsRemoteAbilityCancellation=false, NetSecurityPolicy=ServerOnlyTermination.
 *
 * 스태미나 재생 차단: 활성 동안 Status.StaminaRegen.Blocked 태그를 ActivationOwnedTags로 소유해
 * GE_Stamina_Regen(Ongoing Tag Requirement가 이 태그를 Ignore)의 재생을 즉시 멈춘다.
 * 종료 시에는 ActivationOwnedTags가 곧바로 사라지는 대신, 같은 태그를 부여하는 RecoveryBlockEffectClass를
 * RegenBlockDuration(초) 동안 적용해 "종료 후 N초 지나야 재생 재개"를 만든다. GE가 ActivationOwnedTags보다
 * 먼저 적용되므로 두 소유 구간이 겹쳐 끊김 없이 이어진다.
 */
UCLASS()
class A1_API UA1Ability_Sprint_Active : public UCommonGameplayAbility
{
	GENERATED_BODY()

public:
	UA1Ability_Sprint_Active(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	/** 매 프레임 정지·낙하 여부를 검사하여 조건을 벗어나면 종료한다. */
	UFUNCTION()
	void OnTick(float DeltaTime);

	/** 스프린트 입력을 떼면 종료한다. */
	UFUNCTION()
	void OnInputReleased(float TimeHeld);

	/** 주기마다 스태미나를 소모하고, 소모 실패 시 종료한다. */
	UFUNCTION()
	void OnSprintCommitTick();

	/** 아바타 CharacterMovement의 MaxWalkSpeed를 로컬에서 배율 적용/원복한다. 비복제이므로 실행되는 쪽에서만 적용된다. */
	void SetSprintSpeedLocal(bool bEnable);

protected:
	/**
	 * 종료 시 부여할 회복 억제 GameplayEffect.
	 * Duration Policy를 "Has Duration"으로, Granted Tags에 Status.StaminaRegen.Blocked를 설정해야 한다.
	 * 실제 지속시간은 이 GE 자체의 값이 아니라 아래 RegenBlockDuration으로 코드에서 덮어써 적용된다.
	 */
	UPROPERTY(EditDefaultsOnly, Category="A1|Sprint Active")
	TSubclassOf<UGameplayEffect> RecoveryBlockEffectClass;

	/** 스프린트 종료 후 스태미나 재생이 다시 시작되기까지의 지연 시간(초). */
	UPROPERTY(EditDefaultsOnly, Category="A1|Sprint Active")
	float RegenBlockDuration = 3.f;

	/** 스프린트 중 MaxWalkSpeed에 곱할 배율. */
	UPROPERTY(EditDefaultsOnly, Category="A1|Sprint Active")
	float SprintSpeedMultiplier = 1.3f;

	/** 이 값 이하의 평면 속도면 정지로 간주하여 스프린트를 종료한다. */
	UPROPERTY(EditDefaultsOnly, Category="A1|Sprint Active")
	float StopSpeedThreshold = 10.f;

	/** 스태미나를 소모(CommitCost)하는 주기(초). */
	UPROPERTY(EditDefaultsOnly, Category="A1|Sprint Active")
	float CommitInterval = 0.1f;

private:
	FTimerHandle SprintCommitTimerHandle;

	/** 스프린트 진입 전 MaxWalkSpeed. 종료 시 원복용. */
	float CachedMaxWalkSpeed = 0.f;

	/** 현재 스프린트 배율이 적용돼 있는지. 중복 적용/원복 방지용. */
	bool bSprintSpeedApplied = false;
};
