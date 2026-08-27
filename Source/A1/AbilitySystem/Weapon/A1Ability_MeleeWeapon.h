// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/A1Ability_Equipment.h"
#include "A1Ability_MeleeWeapon.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(A1Ability_MeleeWeapon, Log, All);

class AController;
class UCommonCameraMode;


UCLASS()
class A1_API UA1Ability_MeleeWeapon : public UA1Ability_Equipment
{
	GENERATED_BODY()
	
public:
	UA1Ability_MeleeWeapon(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

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

	/**
	 * 어빌리티 종료 후 스태미나 재생 재개를 지연시키기 위해 부여할 회복 억제 GameplayEffect.
	 * Duration Policy를 "Has Duration"으로, Granted Tags에 Status.StaminaRegen.Blocked를 설정해야 한다.
	 * 실제 지속시간은 이 GE 자체 값이 아니라 아래 RegenBlockDuration으로 코드에서 덮어써 적용된다.
	 * (Sprint의 RecoveryBlockEffectClass와 동일한 GE 에셋을 재사용해도 된다)
	 */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Stamina")
	TSubclassOf<UGameplayEffect> RecoveryBlockEffectClass;

	/** 공격 종료 후 스태미나 재생이 다시 시작되기까지의 지연 시간(초). */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Stamina")
	float RegenBlockDuration = 1.f;

	/**
	 * 이 어빌리티가 활성화되는 동안 카메라를 임시로 확대/축소할 카메라 모드. 비워두면(기본값) 줌 연출이 없다.
	 * 기본 공격은 비워두고, 스킬(GroundBreaker/WhirlwindSlash) BP 데이터에서만 채워 스킬 전용 줌으로 쓴다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Camera")
	TSubclassOf<UCommonCameraMode> ZoomCameraModeClass;
};
