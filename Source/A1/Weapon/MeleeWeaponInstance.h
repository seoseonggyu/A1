// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/WeaponInstance.h"
#include "MeleeWeaponInstance.generated.h"

class UAnimMontage;
class UWeaponViewModel;

DECLARE_LOG_CATEGORY_EXTERN(MeleeWeaponInstanceLog, Log, All);

UCLASS(BlueprintType, Blueprintable)
class A1_API UMeleeWeaponInstance : public UWeaponInstance
{
	GENERATED_BODY()

	friend class UEquipmentComponent;

public:
	UMeleeWeaponInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UAnimMontage* GetAttackMontage(int Index) const 
	{
		if (Index < 0 || AttackMontage.Num() <= Index) return nullptr;
		return AttackMontage[Index]; 
	}

	float GetComboDamage(int Index) const 
	{ 
		if (ComboDamage.Num() < 0) return BaseDamage;
		if (Index < 0 || ComboDamage.Num() <= Index) return BaseDamage;
		
		return ComboDamage[Index]; 
	}

	float GetBaseDamage() const { return BaseDamage; }

	/**
	 * 이 무기로 공격할 때 소비할 스태미나 값. (무기마다 다르며, 밸런싱에 따라 값이 바뀔 수 있다)
	 * UA1Ability_MeleeWeaponAttack이 SetByCaller로 스태미나 소비 GE에 전달한다.
	 */
	float GetStaminaCost() const { return StaminaCost; }

protected:
	virtual void OnEquipped() override;
	virtual void OnUnequipped() override;
	
private:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Animation", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UAnimMontage>> AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Stats", meta = (AllowPrivateAccess = "true"))
	TArray<float> ComboDamage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Stats", meta = (AllowPrivateAccess = "true"))
	float BaseDamage;

	/** 공격 1회당 소비하는 스태미나. 무기별로 다르게 설정하며, 값은 밸런싱에 따라 조정될 수 있다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Stats", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float StaminaCost = 0.f;
};