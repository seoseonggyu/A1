// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/CommonGameplayAbility.h"
#include "A1Ability_Equipment.generated.h"

class UWeaponInstance;
class UMeleeWeaponInstance;

DECLARE_LOG_CATEGORY_EXTERN(A1Ability_Equipment, Log, All);

UCLASS(Abstract)
class A1_API UA1Ability_Equipment: public UCommonGameplayAbility
{
	GENERATED_BODY()

public:
	UA1Ability_Equipment(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;


protected:
	/** Equipment.Slot.Weapon에 장착된 무기 인스턴스를 반환한다. (근접/원거리 공통 베이스 타입) */
	virtual UWeaponInstance* GetWeaponInstance() const;

	/** GetWeaponInstance()를 UMeleeWeaponInstance로 캐스팅한다. 근접 무기 전용 파생 클래스에서 쓴다. */
	virtual UMeleeWeaponInstance* GetMeleeWeaponInstance() const;

};