// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/CommonGameplayAbility.h"
#include "GameplayTagContainer.h"
#include "A1Ability_ChangeQuickBarSlot.generated.h"

class UQuickBarComponent;

/**
 * QuickBar 슬롯 변경 Ability
 *
 * 입력 시 지정된 SlotTag의 슬롯을 활성화합니다.
 * 블루프린트에서 SlotTag를 설정하여 각 숫자키별로 다른 슬롯을 선택하도록 구성합니다.
 */
UCLASS()
class A1_API UA1Ability_ChangeQuickBarSlot : public UCommonGameplayAbility
{
	GENERATED_BODY()

public:
	UA1Ability_ChangeQuickBarSlot(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//-----------------------------------------------------------------------------
	// UGameplayAbility 오버라이드
	//-----------------------------------------------------------------------------

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/** QuickBarComponent를 가져옵니다 */
	UQuickBarComponent* GetQuickBarComponent(const FGameplayAbilityActorInfo* ActorInfo) const;

protected:
	/** 활성화할 QuickBar 슬롯 태그 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "QuickBar", meta = (Categories = "QuickBar.Slot"))
	FGameplayTag SlotTag;
	
	/** 슬롯 내 인덱스 (기본값: 0) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "QuickBar")
	int32 SlotIndex = 0;
};
