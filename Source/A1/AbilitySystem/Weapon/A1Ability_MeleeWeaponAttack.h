// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "A1Ability_MeleeWeapon.h"
#include "A1Ability_MeleeWeaponAttack.generated.h"


class UGameplayEffect;

DECLARE_LOG_CATEGORY_EXTERN(A1Ability_MeleeWeaponAttack, Log, All);


UCLASS()
class A1_API UA1Ability_MeleeWeaponAttack : public UA1Ability_MeleeWeapon
{
	GENERATED_BODY()

public:
	UA1Ability_MeleeWeaponAttack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	virtual void HandleMontageEvent(FGameplayEventData Payload);

private:
	UFUNCTION()
	void OnTargetDataReady(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnMontageEventTriggered(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageFinished();

	/** 아바타 캐릭터의 OrientRotationToMovement를 로컬(서버·소유 클라 각각)에서 설정한다. 비복제 설정이므로 실행되는 쪽에서만 적용된다. */
	void SetOrientRotationToMovementLocal(bool bNewOrient) const;

	/**
	 * 현재 장착 무기의 GetStaminaCost() 값을 SetByCaller로 실어 스태미나 소비 GE를 자신에게 적용한다.
	 * 무기마다 소비량이 다르고 밸런싱으로 바뀔 수 있으므로 GE에 값을 고정하지 않고 SetByCaller로 전달한다.
	 */
	void ApplyStaminaCost() const;

protected:
	UPROPERTY(EditDefaultsOnly, Category="A1|Melee Attack Index")
	uint32 ComboIndex = 0;

	/** true일 때만 공격 중 이동 방향으로의 자동 회전(OrientRotationToMovement)을 끈다. */
	UPROPERTY(EditDefaultsOnly, Category="A1|Melee Attack Rotation")
	bool bDisableOrientRotationDuringAttack = true;

	/**
	 * 스태미나 소비에 사용할 GameplayEffect. (Stamina Attribute에 SetByCaller.StaminaCost로 소비량 전달)
	 * Instant GE로, Stamina에 Additive Modifier + Magnitude를 SetByCaller(SetByCaller.StaminaCost)로 두고
	 * 계수(Coefficient)를 -1로 설정하면 무기별 소비량만큼 스태미나가 감소한다.
	 */
	UPROPERTY(EditDefaultsOnly, Category="A1|Melee Attack Stamina")
	TSubclassOf<UGameplayEffect> StaminaCostEffectClass;

private:
	/** 어빌리티 실행 전 OrientRotationToMovement 값. EndAbility에서 원복하기 위해 캐시한다. */
	bool bCachedOrientRotationToMovement = false;

};