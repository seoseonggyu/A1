// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/WeaponInstance.h"
#include "RangedWeaponInstance.generated.h"

class AA1Projectile;
class UAnimMontage;

DECLARE_LOG_CATEGORY_EXTERN(RangedWeaponInstanceLog, Log, All);

/**
 * URangedWeaponInstance
 *
 * 투사체를 발사하는 원거리 무기(지팡이 등)의 데이터. UMeleeWeaponInstance와 대응되는 원거리 버전으로,
 * 콤보 대신 발사 1회에 필요한 몽타주/투사체 클래스/데미지/스태미나 소비량을 보관한다.
 * 실제 발사 판정은 UA1Ability_RangedWeaponAttack이 담당한다.
 *
 * 차징: 공격 입력을 누르고 있다가 떼는 순간 발사된다(UA1Ability_RangedWeaponAttack이
 * UAbilityTask_WaitInputRelease로 홀드 시간을 측정). BaseDamage/ProjectileSpeed는 0% 충전(즉시 릴리즈)
 * 값이고, MaxCharge* 는 MaxChargeDuration만큼 홀드했을 때(100% 충전) 도달하는 값이다. 그 사이는 선형 보간.
 */
UCLASS(BlueprintType, Blueprintable)
class A1_API URangedWeaponInstance : public UWeaponInstance
{
	GENERATED_BODY()

	friend class UEquipmentComponent;

public:
	URangedWeaponInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UAnimMontage* GetAttackMontage() const { return AttackMontage; }
	TSubclassOf<AA1Projectile> GetProjectileClass() const { return ProjectileClass; }
	FName GetProjectileSocketName() const { return ProjectileSocketName; }
	float GetProjectileSpeed() const { return ProjectileSpeed; }
	float GetBaseDamage() const { return BaseDamage; }

	float GetMaxChargeDuration() const { return MaxChargeDuration; }
	float GetMaxChargeDamage() const { return MaxChargeDamage; }
	float GetMaxChargeSpeed() const { return MaxChargeSpeed; }
	float GetMaxChargeScale() const { return MaxChargeScale; }
	float GetMaxChargeStaminaCost() const { return MaxChargeStaminaCost; }

	/**
	 * 홀드를 시작하는 즉시(0% 충전) 소비하는 기본 스태미나 값.
	 * UA1Ability_RangedWeaponAttack이 SetByCaller로 스태미나 소비 GE에 전달한다.
	 */
	float GetStaminaCost() const { return StaminaCost; }

protected:
	virtual void OnEquipped() override;
	virtual void OnUnequipped() override;

private:
	/** 발사 시 재생할 몽타주. 이 몽타주의 특정 프레임에 UA1AnimNotify_SendGameplayEvent(GameplayEvent.Weapon.Fire)를 배치해 발사 시점을 지정한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Animation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> AttackMontage;

	/** 발사할 투사체 액터 클래스. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Projectile", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AA1Projectile> ProjectileClass;

	/** 투사체를 스폰할 무기 액터(스태프 메시)의 소켓 이름. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Projectile", meta = (AllowPrivateAccess = "true"))
	FName ProjectileSocketName = TEXT("Muzzle");

	/** 투사체 발사 속도(초당 유닛). 0% 충전(즉시 릴리즈) 값. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Projectile", meta = (AllowPrivateAccess = "true"))
	float ProjectileSpeed = 1200.f;

	/** 0% 충전(즉시 릴리즈) 데미지. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Stats", meta = (AllowPrivateAccess = "true"))
	float BaseDamage = 10.f;

	/** 홀드를 시작하는 즉시 소비하는 기본 스태미나(0% 충전 값). 0 이하면 기본 소비가 없다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Stats", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float StaminaCost = 0.f;

	/** 이 시간(초) 이상 홀드하면 최대 충전(100%)에 도달한다. 0이면 차징 없이 항상 0% 충전 값으로 발사한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Charge", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MaxChargeDuration = 1.5f;

	/** 100% 충전 시 데미지. BaseDamage와의 사이를 충전 비율로 선형 보간한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Charge", meta = (AllowPrivateAccess = "true"))
	float MaxChargeDamage = 30.f;

	/** 100% 충전 시 투사체 속도. ProjectileSpeed와의 사이를 충전 비율로 선형 보간한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Charge", meta = (AllowPrivateAccess = "true"))
	float MaxChargeSpeed = 2500.f;

	/** 100% 충전 시 투사체 크기 배수. 0% 충전은 배수 1(원래 크기)이며 그 사이는 선형 보간한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Charge", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float MaxChargeScale = 2.f;

	/**
	 * 100% 충전(MaxChargeDuration만큼 다 홀드)까지 걸리는 동안 총 소비되는 스태미나.
	 * StaminaCost(홀드 시작 즉시 소비되는 기본값)와의 차액을 홀드하는 동안 실시간으로 나눠서 계속 소비한다
	 * (UA1Ability_RangedWeaponAttack의 반복 타이머). 최대 충전에 도달하면 더 이상 소비하지 않는다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Charge", meta = (AllowPrivateAccess = "true"))
	float MaxChargeStaminaCost = 20.f;
};
