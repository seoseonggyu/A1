// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Engine/NetSerialization.h"
#include "AbilitySystem/A1Ability_Equipment.h"
#include "A1Ability_RangedWeaponAttack.generated.h"

class UGameplayEffect;
class URangedWeaponInstance;
class UA1RangedChargeWidget;
class UA1AbilityTask_WaitForTick;

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

	void SetOrientRotationToMovement(bool bRotate);

	/** 매 프레임(WaitForTick) 호출. 조준 방향을 계산해 현재 회전에서 부드럽게(RInterpTo) 보간한다. */
	void UpdateAimRotationLocal(float DeltaTime);

	UFUNCTION()
	void OnAimTick(float DeltaTime);

	/** Fire 이벤트(팔을 내리는 순간) 시점에 AimTickTask를 종료해 더 이상 조준 회전이 갱신되지 않게 한다. */
	void StopAimTickLocal();

	/**
	 * 소유 클라 → 서버로 최신 조준 방향을 전달한다. 서버는 커서 정보가 없는 원격 클라 소유
	 * 캐릭터의 회전을 직접 계산할 수 없으므로(마우스 위치는 소유 클라 로컬 정보), 이 값을 받아
	 * ServerAimDirection에 저장해두고 그 방향으로 회전을 보간한다. 스냅샷 전송이라 Unreliable로 충분하다.
	 */
	UFUNCTION(Server, Unreliable)
	void UpdateAimDirectionServer(FVector_NetQuantizeNormal AimDirection);

	/**
	 * TickCharge/OnFireEventReceived에서 공통으로 호출. 소유 클라(호스트 포함)면 항상 최신 조준
	 * 방향을 서버로 보낸다. 호스트/스탠드얼론처럼 이미 권한이 있는 쪽에서 호출해도, Server RPC는
	 * 네트워크를 타지 않고 그 자리에서 바로 실행되므로 안전하다 — HasAuthority로 걸러내면 안 된다.
	 */
	void SyncAimDirectionToServerLocal();

	UFUNCTION()
	void OnInputReleased(float TimeHeld);

	UFUNCTION()
	void OnFireEventReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageFinished();
	
	void PlayFireMontage();
	
	void SpawnProjectileAuth();

	URangedWeaponInstance* GetRangedWeaponInstance() const;

	void ApplyStaminaCostAmount(float Amount) const;

	void StartChargeTimer();

	void StopChargeTimer();

	void TickCharge();

	void ShowChargeWidgetLocal(float MaxChargeDuration);

	/** 차징이 시간이 아니라 스태미나 고갈로 일찍 멈췄을 때, 위젯 애니메이션을 현재 지점에서 멈춰 실제 ChargeAlpha와 UI를 일치시킨다. */
	void PauseChargeWidgetLocal();

	void HideChargeWidgetLocal();

	UA1RangedChargeWidget* FindChargeWidgetLocal() const;

private:

	float ChargeAlpha = 0.f;
	
	bool bChargeWidgetVisible = false;
	
	static constexpr float ChargeTickInterval = 0.1f;

	FTimerHandle ChargeTickTimerHandle;

	/** StartChargeTimer 이후 지난 시간(초). MaxChargeDuration에 도달하면 타이머를 멈춘다. */
	float ChargeTickElapsed = 0.f;

	/** 현재 차징 타이머가 돌고 있는지. 매 StartChargeTimer 호출마다 리셋되며, 시간·스태미나 상한 중 하나에 도달하면 false로 굳는다. */
	bool bIsCharge = false;

	/**
	 * 아직 소비하지 못하고 이월된 소수점 스태미나. 매 틱 소비량이 1보다 작을 수 있는데
	 * 그때마다 반올림하면 계속 0으로 버려질 수 있어(예: 틱당 0.2), 여기 누적해뒀다가
	 * 1 이상 모이면 그만큼만 정수로 소비하고 나머지를 이월한다.
	 */
	float PendingStaminaFraction = 0.f;

	/** 이번 차징 동안 실제로 소비된 누적 스태미나. MaxChargeStaminaCost 상한 판정과 ChargeAlpha 계산에 쓰인다. */
	float ChargeStaminaConsumed = 0.f;

	/** 소유 클라에서 매 프레임 계산한 최신 조준 방향(정규화). SyncAimDirectionToServerLocal이 이 값을 서버로 전송한다. */
	FVector LocalAimDirection = FVector::ForwardVector;

	/** 서버가 UpdateAimDirectionServer로 전달받은 최신 조준 방향. 서버는 원격 클라 소유 캐릭터를 이 방향으로 회전시킨다. */
	FVector ServerAimDirection = FVector::ForwardVector;

	/** 조준 회전을 매 프레임 갱신하는 태스크. Fire 이벤트 수신 시 StopAimTickLocal에서 EndTask한다. */
	UPROPERTY()
	TObjectPtr<UA1AbilityTask_WaitForTick> AimTickTask;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "A1|Ranged Attack")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "A1|Ranged Attack")
	TSubclassOf<UGameplayEffect> StaminaCostEffectClass;

	/** 조준 회전 보간 속도(FMath::RInterpTo의 InterpSpeed). 클수록 커서를 더 빨리 따라간다. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Ranged Attack")
	float AimRotationInterpSpeed = 20.f;
};
