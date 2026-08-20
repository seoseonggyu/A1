// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Consumable/A1Ability_Consume.h"
#include "A1Ability_DrinkPotion.generated.h"

class UAnimMontage;

DECLARE_LOG_CATEGORY_EXTERN(A1Ability_DrinkPotionLog, Log, All);

/**
 * UA1Ability_DrinkPotion
 *
 * 포션을 마셔 HP/Mana를 회복하는 소비 어빌리티.
 * 손에 든 포션 장비의 FEquipmentFragment_Consume에서 회복 GE(ConsumeEffectClass)를 읽어 소유자에게 적용하고,
 * 마시기 몽타주가 끝나면 원본 아이템을 1개 소비한다. (HP 포션·MP 포션이 각자 다른 GE를 지정한다)
 *
 */
UCLASS()
class A1_API UA1Ability_DrinkPotion : public UA1Ability_Consume
{
	GENERATED_BODY()

public:
	UA1Ability_DrinkPotion(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	/** 마시기 완료(정상 종료/블렌드 아웃) 시: 회복 GE 적용 + 아이템 소비 후 종료. */
	UFUNCTION()
	void OnDrinkCompleted();

	/** 마시기 취소(중단/인터럽트) 시: 소비하지 않고 종료. */
	UFUNCTION()
	void OnDrinkCancelled();

	/** 손에 든 포션의 ConsumeEffectClass(회복 GE)를 소유자에게 적용한다. (서버 권한 전용) */
	void ApplyConsumeEffect();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "A1|Drink Potion")
	TObjectPtr<UAnimMontage> DrinkMontage;
};
