// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CommonAbilityTypes.h"
#include "CommonGameplayAbility.generated.h"

class UCommonAbilitySystemComponent;
class ACommonCharacter;
class UCommonCameraMode;

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
	
	UFUNCTION(BlueprintCallable, Category = "A1Ability")
	ACommonPlayerController* GetCommonPlayerControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "A1|Ability")
	AController* GetControllerFromActorInfo() const;
	
	UFUNCTION(BlueprintCallable, Category = "A1|Ability")
	ACommonCharacter* GetCommonCharacterFromActorInfo() const;
	
	//-----------------------------------------------------------------------------
	// 활성화 정책
	//-----------------------------------------------------------------------------

	/** 활성화 정책을 반환합니다 */
	ECommonAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }

	/** 활성화 그룹을 반환합니다 */
	ECommonAbilityActivationGroup GetActivationGroup() const { return ActivationGroup; }

	//-----------------------------------------------------------------------------
	// 카메라 모드
	//-----------------------------------------------------------------------------

	/** 이 Ability가 활성화되어 있는 동안 카메라 모드를 임시로 덮어쓴다. EndAbility에서 자동으로 해제된다. */
	void SetCameraMode(TSubclassOf<UCommonCameraMode> CameraModeClass);

	/** SetCameraMode로 건 카메라 모드 오버라이드를 해제한다. */
	void ClearCameraMode();

protected:
	//-----------------------------------------------------------------------------
	// UGameplayAbility 오버라이드
	//-----------------------------------------------------------------------------

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/** 태그 관계 매핑으로 확장된 활성화 필수/차단 태그까지 검사한다. */
	virtual bool DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

private:
	/** SetCameraMode로 현재 걸어둔 카메라 모드(없으면 nullptr). ClearCameraMode에서 걸어둔 적이 있는지 확인하는 용도. */
	TSubclassOf<UCommonCameraMode> ActiveCameraModeClass;

protected:
	/** 활성화 정책 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Activation")
	ECommonAbilityActivationPolicy ActivationPolicy = ECommonAbilityActivationPolicy::OnInputTriggered;

	/** 활성화 그룹 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Activation")
	ECommonAbilityActivationGroup ActivationGroup = ECommonAbilityActivationGroup::Independent;
};