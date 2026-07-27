// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/CommonGameplayAbility.h"
#include "A1Ability_Equipment.generated.h"

class UMeleeWeaponInstance;

DECLARE_LOG_CATEGORY_EXTERN(A1Ability_Equipment, Log, All);

UCLASS(Abstract)
class A1_API UA1Ability_Equipment: public UCommonGameplayAbility
{
	GENERATED_BODY()

public:
	UA1Ability_Equipment(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;


protected:
	virtual UMeleeWeaponInstance* GetMeleeWeaponInstance() const;

};