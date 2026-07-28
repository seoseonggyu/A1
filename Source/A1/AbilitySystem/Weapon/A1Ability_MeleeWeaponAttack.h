// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/A1Ability_Equipment.h"
#include "A1Ability_MeleeWeaponAttack.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(A1Ability_MeleeWeaponAttack, Log, All);


UCLASS()
class A1_API UA1Ability_MeleeWeaponAttack : public UA1Ability_Equipment
{
	GENERATED_BODY()

public:
	UA1Ability_MeleeWeaponAttack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	virtual void HandleMontageEvent(FGameplayEventData Payload);
	
private:
	UFUNCTION()
	void OnMontageEventTriggered(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnMontageFinished();
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="A1|Melee Attack Index")
	uint32 ComboIndex = 0;

};