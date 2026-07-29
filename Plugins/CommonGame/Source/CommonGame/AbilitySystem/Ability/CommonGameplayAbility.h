// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CommonAbilityTypes.h"
#include "CommonGameplayAbility.generated.h"

class UCommonAbilitySystemComponent;

/**
 * CommonGame의 기본 GameplayAbility
 *
 * Lyra 스타일 ActivationPolicy와 ActivationGroup을 지원합니다.
 * 입력 태그 기반 활성화 시스템과 연동됩니다.
 */
UCLASS(Abstract)
class COMMONGAME_API UCommonGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UCommonGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "A1|Ability")
	UCommonAbilitySystemComponent* GetCommonAbilitySystemComponentFromActorInfo() const;
	
	//-----------------------------------------------------------------------------
	// 활성화 정책
	//-----------------------------------------------------------------------------

	/** 활성화 정책을 반환합니다 */
	ECommonAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }

	/** 활성화 그룹을 반환합니다 */
	ECommonAbilityActivationGroup GetActivationGroup() const { return ActivationGroup; }

protected:
	//-----------------------------------------------------------------------------
	// UGameplayAbility 오버라이드
	//-----------------------------------------------------------------------------

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	/** 태그 관계 매핑으로 확장된 활성화 필수/차단 태그까지 검사한다. */
	virtual bool DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	/** 활성화 정책 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Activation")
	ECommonAbilityActivationPolicy ActivationPolicy = ECommonAbilityActivationPolicy::OnInputTriggered;

	/** 활성화 그룹 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Activation")
	ECommonAbilityActivationGroup ActivationGroup = ECommonAbilityActivationGroup::Independent;
};