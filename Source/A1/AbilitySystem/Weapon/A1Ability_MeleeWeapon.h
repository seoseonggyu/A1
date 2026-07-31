// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/A1Ability_Equipment.h"
#include "A1Ability_MeleeWeapon.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(A1Ability_MeleeWeapon, Log, All);

class AController;


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

	/**
	 * 플레이어의 이동 "입력"만 로컬(서버·소유 클라 각각)에서 차단/해제한다. 비복제 처리라 실행되는 쪽에서만 적용된다.
	 * 이동 모드는 MOVE_Walking으로 유지되므로 몽타주 루트 모션이나 RootMotionSource(AbilityTask_ApplyRootMotion*)는
	 * 정상적으로 캐릭터를 움직인다. 시전 중 WASD 조작은 막되 애니메이션이 전진시키는 스킬에서 사용한다.
	 */
	void SetMoveInputBlockedLocal(bool bBlocked);

protected:
	UPROPERTY()
	TSet<TWeakObjectPtr<AActor>> CachedHitActors;

	/** SetMoveInputBlockedLocal로 이동 입력을 막아둔 컨트롤러. 차단/해제 짝을 맞추기 위해 기억한다. (유효하면 차단 중) */
	UPROPERTY()
	TWeakObjectPtr<AController> MoveInputBlockedController;
	
	/** 데미지 적용에 사용할 GameplayEffect (Damage Attribute에 SetByCaller로 값 전달) */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass; // TODO: 매번 넣기 보다는 데이터로
};
