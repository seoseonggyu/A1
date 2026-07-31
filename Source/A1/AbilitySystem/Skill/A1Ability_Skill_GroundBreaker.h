#pragma once
#include "AbilitySystem/Weapon/A1Ability_MeleeWeapon.h"

#include "A1Ability_Skill_GroundBreaker.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1Ability_Skill_GroundBreakerLog, Log, All);

/**
 * 내려찍기(Ground Breaker) 스킬.
 * 시전 중에는 WASD 이동 입력만 차단하고 이동 모드는 유지하므로, 몽타주의 루트 모션이 캐릭터를 전진시킨다.
 * 몽타주에 루트 모션이 없다면 ForwardMoveDistance를 0보다 크게 설정해 C++ RootMotionSource로 전진시킬 수 있다.
 */
UCLASS()
class UA1Ability_Skill_GroundBreaker : public UA1Ability_MeleeWeapon
{
	GENERATED_BODY()

public:
	UA1Ability_Skill_GroundBreaker(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION()
	void OnGroundBreakerBegin(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageFinished();

private:
	void ExecuteGroundBreaker();

	/** ForwardMoveDistance가 설정되어 있으면 RootMotionSource로 캐릭터를 정면으로 밀어준다. */
	void StartForwardRootMotion();

	/** bDrawDebugShape가 켜져 있으면 충돌 판정 박스를 그린다. (서버·클라이언트 공용) */
	void DrawDebugShape(const FVector& Center, const FVector& HalfSize, const FRotator& Orientation, bool bHasHit) const;

protected:
	UPROPERTY(EditDefaultsOnly, Category="A1|Ground Breaker")
	TObjectPtr<UAnimMontage> GroundBreakerMontage;

	UPROPERTY(EditDefaultsOnly, Category="A1|Ground Breaker")
	float DistanceOffset = 150.f;

	UPROPERTY(EditDefaultsOnly, Category="A1|Ground Breaker")
	float Damage = 50.f;

	UPROPERTY(EditDefaultsOnly, Category="A1|Ground Breaker")
	float StunDruation = 3.f;

	//--------------------------------------------------------------------------
	// 전진 이동 (몽타주에 루트 모션이 없을 때만 사용하는 대체 수단)
	//--------------------------------------------------------------------------

	/** 시전 시작 시 정면으로 밀어낼 거리(cm). 0 이하이면 사용하지 않고 몽타주 루트 모션에만 의존한다. */
	UPROPERTY(EditDefaultsOnly, Category="A1|Ground Breaker|Movement", meta=(ClampMin="0.0"))
	float ForwardMoveDistance = 0.f;

	/** 위 거리를 이동하는 데 걸리는 시간(초). 몽타주에서 실제로 전진하는 구간 길이에 맞춘다. */
	UPROPERTY(EditDefaultsOnly, Category="A1|Ground Breaker|Movement", meta=(ClampMin="0.01"))
	float ForwardMoveDuration = 0.3f;

	//--------------------------------------------------------------------------
	// 디버그
	//--------------------------------------------------------------------------

	/** true면 ExecuteGroundBreaker의 충돌 판정 박스를 그린다. 서버·클라이언트 양쪽에서 그려진다. (BP 클래스 기본값에서 수정 가능) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="A1|Ground Breaker|Debug")
	bool bDrawDebugShape = false;

	/** 디버그 박스 표시 시간(초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="A1|Ground Breaker|Debug", meta=(EditCondition="bDrawDebugShape"))
	float DebugDrawDuration = 2.f;

	/** 히트가 없을 때(그리고 히트 여부를 모르는 클라이언트) 색상 */
	UPROPERTY(EditDefaultsOnly, Category="A1|Ground Breaker|Debug", meta=(EditCondition="bDrawDebugShape"))
	FColor TraceColor = FColor::Red;

	/** 히트가 있을 때 색상 (서버 전용) */
	UPROPERTY(EditDefaultsOnly, Category="A1|Ground Breaker|Debug", meta=(EditCondition="bDrawDebugShape"))
	FColor HitColor = FColor::Green;
};
