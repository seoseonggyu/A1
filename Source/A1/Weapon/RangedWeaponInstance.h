// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/WeaponInstance.h"
#include "RangedWeaponInstance.generated.h"

class AA1Projectile;
class AA1SkillAOEZone;
class UAnimMontage;
class UGameplayEffect;

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
 *
 * AOE 스킬(UA1Ability_Skill_AOE)용 장판 수치도 함께 들고 있다. 어빌리티는 장착된 아이템마다
 * FEquipmentFragment_Ability로 개별 부여되지만 클래스 자체는 지팡이 전부가 공유하므로, 데미지/슬로우
 * 수치는 물론 적용할 GameplayEffect 클래스까지 전부 이 무기 인스턴스 데이터로 두어 지팡이마다
 * (동일 어빌리티 클래스라도) 서로 다른 피해량·이펙트를 가질 수 있게 한다.
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

	TSubclassOf<AA1SkillAOEZone> GetAOEZoneClass() const { return AOEZoneClass; }
	float GetAOERadius() const { return AOERadius; }
	float GetAOETickDamage() const { return AOETickDamage; }
	float GetAOETickInterval() const { return AOETickInterval; }
	int32 GetAOETickCount() const { return AOETickCount; }
	float GetAOESlowAmount() const { return AOESlowAmount; }
	float GetAOESlowDuration() const { return AOESlowDuration; }
	TSubclassOf<UGameplayEffect> GetAOEDamageEffectClass() const { return AOEDamageEffectClass; }
	TSubclassOf<UGameplayEffect> GetAOESlowEffectClass() const { return AOESlowEffectClass; }
	UAnimMontage* GetAOECastMontage() const { return AOECastMontage; }
	UAnimMontage* GetAOESkillMontage() const { return AOESkillMontage; }

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

	//-----------------------------------------------------------------------------
	// AOE 스킬(UA1Ability_Skill_AOE) 전용
	//-----------------------------------------------------------------------------

	/** 커서 위치에 스폰할 장판 액터. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|AOE", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AA1SkillAOEZone> AOEZoneClass;

	/** 장판 반경(cm). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|AOE", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float AOERadius = 300.f;

	/** 틱 1회당(1초마다) 입히는 데미지. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|AOE", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float AOETickDamage = 10.f;

	/** 데미지 틱 주기(초). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|AOE", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float AOETickInterval = 1.f;

	/** 총 틱 횟수(AOETickInterval * AOETickCount = 장판 지속시간). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|AOE", meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	int32 AOETickCount = 3;

	/** 틱마다 MoveSpeedMultiplier에서 깎는 양(0~1). 0.5면 이동속도 50% 감소. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|AOE", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float AOESlowAmount = 0.5f;

	/** 슬로우 지속시간(초). AOETickInterval보다 살짝 길게 잡아 장판 안에 머무는 동안 끊기지 않게 한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|AOE", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float AOESlowDuration = 1.5f;

	/** 장판 데미지 틱에 쓰이는 Instant GE (SetByCaller.BaseDamage). 지팡이마다 다른 피해 GE를 쓸 수 있도록 무기 데이터로 둔다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|AOE", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> AOEDamageEffectClass;

	/** 슬로우에 쓰이는 Duration GE (SetByCaller.SlowAmount로 MoveSpeedMultiplier를 깎음). 지팡이마다 다르거나 아예 없을 수 있다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|AOE", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> AOESlowEffectClass;

	/**
	 * 조준(캐스팅 시작) 중 재생하는 몽타주. 다른 플레이어에게 "이 캐릭터가 스킬을 준비 중"임을
	 * 알리는 것이 목적이라 확정 전까지 반복되도록 몽타주 자체를 루프 섹션으로 구성해두는 것이 좋다.
	 * 비워두면 조준 중 재생되는 몽타주 없이 진행된다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|AOE|Animation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> AOECastMontage;

	/** 확정 순간 재생하는 실제 스킬 발동 몽타주. 재생이 끝나면(Completed/BlendOut/Interrupted/Cancelled 모두) 어빌리티가 종료된다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|AOE|Animation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> AOESkillMontage;
};
