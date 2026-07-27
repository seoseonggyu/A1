// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "A1Ability_MeleeWeaponAttack.h"
#include "A1Ability_MeleeWeaponComboAttack.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(A1Ability_MeleeWeaponComboAttack, Log, All);


UCLASS()
class A1_API UA1Ability_MeleeWeaponComboAttack : public UA1Ability_MeleeWeaponAttack
{
	GENERATED_BODY()

public:
	UA1Ability_MeleeWeaponComboAttack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	virtual void HandleMontageEvent(FGameplayEventData Payload); /*TEMP*/
	
private:
	void WaitInputContinue();
	void WaitInputStop();
	
private:
	UFUNCTION()
	void OnInputRelease(float TimeHeld);
	
	UFUNCTION()
	void OnInputStart();
	
	UFUNCTION()
	void OnInputCancel();
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="A1|Melee Combo")
	TSubclassOf<UA1Ability_MeleeWeaponComboAttack> NextAbilityClass;
	
private:
	bool bInputPressed = false;
	bool bInputReleased = false;
};
