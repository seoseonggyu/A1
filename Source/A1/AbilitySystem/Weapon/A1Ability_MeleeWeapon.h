// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/A1Ability_Equipment.h"
#include "A1Ability_MeleeWeapon.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(A1Ability_MeleeWeapon, Log, All);


UCLASS()
class A1_API UA1Ability_MeleeWeapon : public UA1Ability_Equipment
{
	GENERATED_BODY()
	
public:
	UA1Ability_MeleeWeapon(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UFUNCTION()
	void ParseTargetData(const FGameplayAbilityTargetDataHandle& InTargetDataHandle, TArray<int32>& OutCharacterHitIndexes);

	UFUNCTION()
	void ProcessHitResult(const FHitResult& HitResult, float Damage) const;
	
	UFUNCTION()
	void ResetHitActors();

	/** 아바타 캐릭터의 이동을 로컬(서버·소유 클라 각각)에서 정지/복구한다. 비복제 이동 모드 변경이므로 실행되는 쪽에서만 적용된다. */
	void SetMovementFrozenLocal(bool bFrozen) const;

protected:
	UPROPERTY()
	TSet<TWeakObjectPtr<AActor>> CachedHitActors;
	
	/** 데미지 적용에 사용할 GameplayEffect (Damage Attribute에 SetByCaller로 값 전달) */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass; // TODO: 매번 넣기 보다는 데이터로
};
