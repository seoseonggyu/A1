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
};