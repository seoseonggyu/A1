// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/WeaponInstance.h"
#include "MeleeWeaponInstance.generated.h"

class UAnimMontage;
class UWeaponViewModel;

DECLARE_LOG_CATEGORY_EXTERN(MeleeWeaponInstanceLog, Log, All);

/**
 * 근거리 무기 인스턴스
 *
 * WeaponInstance를 관리 기능을 추가합니다.
 */
UCLASS(BlueprintType, Blueprintable)
class A1_API UMeleeWeaponInstance : public UWeaponInstance
{
	GENERATED_BODY()

	friend class UEquipmentComponent;

public:
	UMeleeWeaponInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void OnEquipped() override;
	virtual void OnUnequipped() override;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Animation")
	TArray<TObjectPtr<UAnimMontage>> AttackMontage;

private:

};