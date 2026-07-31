#pragma once

#include "AbilitySystem/Ability/CommonGameplayAbility.h"
#include "AttributeSet.h"
#include "A1Ability_Sprint_Check.generated.h"

/**
 * UA1Ability_Sprint_Check
 *
 * 스프린트 입력(Input.Ability.Sprint)이 들어온 순간 1회 실행되어 스프린트 가능 여부만 판정하는 게이트 어빌리티.
 * 조건(지면 위 · 이동 중 · 스태미나 충분)을 만족하면 실제 지속 로직인 UA1Ability_Sprint_Active를
 * GameplayEvent(Ability.Sprint.Active)로 트리거하고 곧바로 종료한다.
 *
 * 탑뷰 특성상 캐릭터가 이동 방향으로 자동 회전(bOrientRotationToMovement)하므로
 * '전방(Forward) 이동' 판정 대신 '이동 입력 존재' 여부로 판정한다.
 *
 * 네트워크: 입력을 소유한 쪽에서만 판정하며, 소유 클라는 로컬 예측 발동과 함께
 * 서버로 권위 발동을 요청(RequestSprintServer)한다. 호스트/스탠드얼론은 로컬에서 바로 발동한다.
 */
UCLASS()
class A1_API UA1Ability_Sprint_Check : public UCommonGameplayAbility
{
	GENERATED_BODY()

public:
	UA1Ability_Sprint_Check(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	/** 소유 클라 → 서버. 서버에서 Active 이벤트를 권위적으로 쏘고 Check를 종료한다. */
	UFUNCTION(Server, Reliable)
	void RequestSprintServer();

	/** 호출한 쪽(비복제)에서 Active 어빌리티를 트리거하는 GameplayEvent를 발송한다. */
	void SendSprintActiveEventLocal();

protected:
	/** 스프린트 시작 가능 여부를 검사할 스태미나 계열 Attribute. 기본값은 UA1VitalSet::Stamina. */
	UPROPERTY(EditDefaultsOnly, Category="A1|Sprint Check")
	FGameplayAttribute SprintCostAttribute;

	/** 이 값 이하로 스태미나가 남아 있으면 스프린트를 시작하지 않는다. */
	UPROPERTY(EditDefaultsOnly, Category="A1|Sprint Check")
	float MinStaminaToStart = 10.f;
};
