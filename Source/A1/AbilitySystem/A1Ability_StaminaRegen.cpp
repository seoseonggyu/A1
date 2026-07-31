// Copyright Epic Games, Inc. All Rights Reserved.

#include "A1Ability_StaminaRegen.h"

#include "GameplayEffect.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_StaminaRegen)

DEFINE_LOG_CATEGORY(A1Ability_StaminaRegenLog);

UA1Ability_StaminaRegen::UA1Ability_StaminaRegen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 스폰(부여) 즉시 자동 활성화되는 패시브.
	ActivationPolicy = ECommonAbilityActivationPolicy::OnSpawn;
}

void UA1Ability_StaminaRegen::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 무한 재생 GE는 서버에서만 적용한다. (클라로 복제되므로 예측/중복 적용이 불필요)
	if (HasAuthority(&ActivationInfo) && RegenEffectClass)
	{
		const UGameplayEffect* GameplayEffectCDO = RegenEffectClass->GetDefaultObject<UGameplayEffect>();
		(void)ApplyGameplayEffectToOwner(Handle, ActorInfo, ActivationInfo, GameplayEffectCDO, 1);
	}

	// GE는 어빌리티 수명과 독립적으로 유지되므로 적용 후 바로 종료한다.
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
