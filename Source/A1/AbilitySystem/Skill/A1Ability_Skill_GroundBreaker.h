#pragma once
#include "AbilitySystem/Weapon/A1Ability_MeleeWeapon.h"

#include "A1Ability_Skill_GroundBreaker.generated.h"

UCLASS()
class UA1Ability_Skill_GroundBreaker : public UA1Ability_MeleeWeapon
{
	GENERATED_BODY()

public:
	UA1Ability_Skill_GroundBreaker(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION()
	void OnGroundBreakerBegin(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageFinished();

private:
	void ExecuteGroundBreaker();

	/** 아바타 캐릭터의 이동을 로컬(서버·소유 클라 각각)에서 정지/복구한다. 비복제 이동 모드 변경이므로 실행되는 쪽에서만 적용된다. */
	void SetMovementFrozenLocal(bool bFrozen) const;

protected:
	UPROPERTY(EditDefaultsOnly, Category="A1|Ground Breaker")
	TObjectPtr<UAnimMontage> GroundBreakerMontage;

	UPROPERTY(EditDefaultsOnly, Category="A1|Ground Breaker")
	float DistanceOffset = 150.f;

	UPROPERTY(EditDefaultsOnly, Category="A1|Ground Breaker")
	float Damage = 50.f;

	UPROPERTY(EditDefaultsOnly, Category="A1|Ground Breaker")
	float StunDruation = 3.f;
};