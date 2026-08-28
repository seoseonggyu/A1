// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "AbilitySystem/A1Ability_Equipment.h"
#include "A1Ability_RangedWeaponAttack.generated.h"

class UGameplayEffect;
class URangedWeaponInstance;
class UA1RangedChargeWidget;

DECLARE_LOG_CATEGORY_EXTERN(A1Ability_RangedWeaponAttackLog, Log, All);


UCLASS()
class A1_API UA1Ability_RangedWeaponAttack : public UA1Ability_Equipment
{
	GENERATED_BODY()

public:
	UA1Ability_RangedWeaponAttack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	UFUNCTION()
	void OnInputReleased(float TimeHeld);

	UFUNCTION()
	void OnFireEventReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageFinished();

	/** 입력이 풀린 뒤 충전 비율이 확정된 상태에서 발사 몽타주를 재생한다. */
	void PlayFireMontage();

	/** GameplayEvent.Weapon.Fire를 받았을 때 실행된다. 서버 인스턴스에서만 실제로 투사체를 스폰한다. */
	void SpawnProjectileAuth();

	URangedWeaponInstance* GetRangedWeaponInstance() const;

	/** Amount(양수)만큼 SetByCaller로 실어 스태미나 소비 GE를 자신에게 적용한다. 홀드 시작 즉시 1회, 이후 반복 타이머에서 조금씩 여러 번 호출된다. */
	void ApplyStaminaCostAmount(float Amount) const;

	/** 차징 반복 타이머를 시작한다(경과 시간·이월 소수점 초기화 포함). */
	void StartChargeTimer();

	/** 차징 반복 타이머를 멈춘다(이미 멈춰 있으면 아무 일도 하지 않음). */
	void StopChargeTimer();

	/** ChargeTickInterval마다 호출되어, 조준 회전 갱신과 스태미나 소비를 함께 처리한다. 최대 충전에 도달하면 스스로 타이머를 멈춘다. */
	void TickCharge();

	/** 소유 클라 전용: 마우스 커서가 가리키는 지점 방향을 계산해 UpdateAimDirectionServer로 서버에 전달한다. 캐릭터/컨트롤러 회전은 건드리지 않는다. */
	void UpdateAimDirectionLocal();

	/** 클라 → 서버 RPC. 자주 호출되므로 Unreliable(최신 값만 중요). ServerAimDirection을 갱신한다. */
	UFUNCTION(Server, Unreliable)
	void UpdateAimDirectionServer(FVector_NetQuantizeNormal InAimDirection);

	/** 소유 클라 전용: 차징 UI를 보여주고 MaxChargeDuration에 맞춘 재생 속도로 재생을 시작한다. */
	void ShowChargeWidgetLocal(float MaxChargeDuration);

	/** 소유 클라 전용: 차징 UI를 제거한다(발사/취소 무관, 이미 안 보이면 아무 일도 하지 않음). */
	void HideChargeWidgetLocal();

	/** 소유 클라 전용: HUD에 상시 배치된 UA1RangedChargeWidget 인스턴스를 찾는다. (UA1Ability_Interact_Hold::FindHoldWidgetLocal과 동일한 방식) */
	UA1RangedChargeWidget* FindChargeWidgetLocal() const;

	/** 플레이어의 이동 방향 자동 회전만 로컬(서버·소유 클라 각각)에서 켜고 끈다. (UA1Ability_MeleeWeapon::SetOrientRotationToMovementLocal과 동일) */
	void SetOrientRotationToMovementLocal(bool bNewOrient) const;

private:
	/** OnInputReleased에서 확정되는 충전 비율(0~1). SpawnProjectileAuth의 데미지/속도/크기 보간에 쓰인다. */
	float ChargeAlpha = 0.f;

	/** ShowChargeWidgetLocal을 실제로 호출했는지 여부. 대상 검증 실패 등으로 위젯을 띄우기 전에 EndAbility가 호출될 수 있어, 불필요한 탐색을 피하려고 둔다. */
	bool bChargeWidgetVisible = false;

	/** 차징 중 조준 회전 갱신 + 스태미나 소비를 처리하는 반복 타이머 간격(초). */
	static constexpr float ChargeTickInterval = 0.1f;

	FTimerHandle ChargeTickTimerHandle;

	/** StartChargeTimer 이후 지난 시간(초). MaxChargeDuration에 도달하면 타이머를 멈춘다. */
	float ChargeTickElapsed = 0.f;

	/**
	 * 아직 소비하지 못하고 이월된 소수점 스태미나. 매 틱 소비량이 1보다 작을 수 있는데
	 * 그때마다 반올림하면 계속 0으로 버려질 수 있어(예: 틱당 0.2), 여기 누적해뒀다가
	 * 1 이상 모이면 그만큼만 정수로 소비하고 나머지를 이월한다.
	 */
	float PendingStaminaFraction = 0.f;

	/** 어빌리티 실행 전 OrientRotationToMovement 값. EndAbility에서 원복하기 위해 캐시한다. */
	bool bCachedOrientRotationToMovement = false;

	/**
	 * 서버가 신뢰하는 조준 방향(수평, 정규화). UpdateAimDirectionServer로 갱신되며
	 * SpawnProjectileAuth의 발사 방향에 쓰인다. 첫 틱 전(예: 즉시 릴리즈) 대비 ActivateAbility에서
	 * 아바타의 현재 정면 방향으로 초기화해둔다.
	 */
	FVector ServerAimDirection = FVector::ForwardVector;

protected:
	/** true면 홀드 시작부터 어빌리티가 끝날 때까지 이동 방향으로의 자동 회전(OrientRotationToMovement)을 끈다. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Ranged Attack")
	bool bDisableOrientRotationDuringCharge = true;

	/** 투사체가 대상에게 적용할 데미지 GameplayEffect. (Damage Attribute에 SetByCaller.BaseDamage로 값 전달) */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Ranged Attack")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/**
	 * 스태미나 소비에 사용할 GameplayEffect. (Stamina Attribute에 SetByCaller.StaminaCost로 소비량 전달)
	 * UA1Ability_MeleeWeaponAttack의 StaminaCostEffectClass와 동일한 GE 에셋을 재사용해도 된다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Ranged Attack")
	TSubclassOf<UGameplayEffect> StaminaCostEffectClass;
};
