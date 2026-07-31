// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Ability/CommonGameplayAbility.h"
#include "A1Ability_StaminaRegen.generated.h"

class UGameplayEffect;

DECLARE_LOG_CATEGORY_EXTERN(A1Ability_StaminaRegenLog, Log, All);

/**
 * UA1Ability_StaminaRegen
 *
 * 스폰 시 자동 활성화(OnSpawn)되어 스태미나 재생용 무한 지속 GameplayEffect를 소유자에게 1회 적용하는 패시브 어빌리티.
 * 실제 회복은 GE가 주기적으로 수행하며(값 클램프는 UA1VitalSet::PreAttributeChange), 어빌리티는 적용만 하고 즉시 종료한다.
 *
 * 재생 GE에 "Status.Sprint를 Ignore하는 Ongoing 태그 요구"를 걸어두면 스프린트 중에는 재생이 자동으로 멈춘다.
 * GE는 서버에서만 적용하며(무한 GE는 클라로 복제됨), 별도 입력 없이 GameFeatureAction_AddAbilities로 부여한다.
 */
UCLASS()
class A1_API UA1Ability_StaminaRegen : public UCommonGameplayAbility
{
	GENERATED_BODY()

public:
	UA1Ability_StaminaRegen(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/** 스태미나 재생용 무한 지속 GameplayEffect. 주기(Period)마다 Stamina를 회복시킨다. */
	UPROPERTY(EditDefaultsOnly, Category="A1|Stamina Regen")
	TSubclassOf<UGameplayEffect> RegenEffectClass;
};
