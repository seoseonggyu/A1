// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "A1Ability_Equipment.h"
#include "A1Ability_MeleeWeaponAttack.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(A1Ability_MeleeWeaponAttack, Log, All);

/**
 * 원거리 무기 사격 Ability
 *
 * 클라이언트에서 히트스캔 후 TargetData를 서버로 전송하고,
 * 서버에서 리와인딩 검증 후 데미지를 적용합니다.
 */
UCLASS()
class A1_API UA1Ability_MeleeWeaponAttack : public UA1Ability_Equipment
{
	GENERATED_BODY()

public:
	UA1Ability_MeleeWeaponAttack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;


private:
	int32 ComboIndex = 0;
	bool bCanNextCombo = true;

};