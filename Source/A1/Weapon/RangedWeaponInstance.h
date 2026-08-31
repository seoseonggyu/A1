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
	float GetMinStaminaToStart() const { return MinStaminaToStart; }

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
	float ProjectileSpeed = 600.f;

	/** 0% 충전(즉시 릴리즈) 데미지. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Stats", meta = (AllowPrivateAccess = "true"))
	float BaseDamage = 10.f;

	/**
	 * 발동 자체에 필요한 최소 스태미나. 소비되지는 않고 "이 이상 있어야 홀드를 시작할 수 있다"는
	 * 문턱값으로만 쓰인다(실제 소비는 0에서 시작해 차징하며 진행). 리젠으로 막 회복돼 얼마 안 되는
	 * 스태미나로 바로 발동해버리는 것을 막기 위한 값이다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Stats", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MinStaminaToStart = 10.f;

	/** 이 시간(초) 이상 홀드하면 최대 충전(100%)에 도달한다. 0이면 차징 없이 항상 0% 충전 값으로 발사한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Charge", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MaxChargeDuration = 1.5f;

	/** 100% 충전 시 데미지. BaseDamage와의 사이를 충전 비율로 선형 보간한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Charge", meta = (AllowPrivateAccess = "true"))
	float MaxChargeDamage = 30.f;

	/** 100% 충전 시 투사체 속도. ProjectileSpeed와의 사이를 충전 비율로 선형 보간한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Charge", meta = (AllowPrivateAccess = "true"))
	float MaxChargeSpeed = 1200.f;

	/** 100% 충전 시 투사체 크기 배수. 0% 충전은 배수 1(원래 크기)이며 그 사이는 선형 보간한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Charge", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float MaxChargeScale = 2.f;

	/** 100% 충전(MaxChargeDuration만큼 다 홀드)했을 때 소비되는 스태미나 총량	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Charge", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float MaxChargeStaminaCost = 20.f;
};
