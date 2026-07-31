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