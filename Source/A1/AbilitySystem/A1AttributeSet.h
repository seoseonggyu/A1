// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/CommonAttributeSet.h"
#include "A1AttributeSet.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1AttributeSetLog, Log, All);

/**
 * 캐릭터 기본 스탯 AttributeSet
 *
 */
UCLASS()
class A1_API UA1AttributeSet : public UCommonAttributeSet
{
	GENERATED_BODY()

public:
	UA1AttributeSet();

	//-----------------------------------------------------------------------------
	// UObject 오버라이드
	//-----------------------------------------------------------------------------

	virtual void BeginDestroy() override;

	//-----------------------------------------------------------------------------
	// UAttributeSet 오버라이드
	//-----------------------------------------------------------------------------

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	//-----------------------------------------------------------------------------
	// UCommonAttributeSet 오버라이드
	//-----------------------------------------------------------------------------

	virtual void PostNetInit() override;

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	/** 초기화된 ViewModel 캐시 (BeginDestroy에서 정리용) */
	UPROPERTY()
	TObjectPtr<class UCharacterViewModel> CachedViewModel;

public:
	//-----------------------------------------------------------------------------
	// Health Attributes
	//-----------------------------------------------------------------------------

	/** 현재 체력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health = 100.f;
	ATTRIBUTE_ACCESSORS(UA1AttributeSet, Health)

	/** 최대 체력 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_MaxHealth, meta = (ShowOnlyInnerProperties))
	FGameplayAttributeData MaxHealth = 100.f;
	ATTRIBUTE_ACCESSORS(UA1AttributeSet, MaxHealth)

	//-----------------------------------------------------------------------------
	// Meta Attributes (복제되지 않음, 계산용)
	//-----------------------------------------------------------------------------
		
	/** 들어온 데미지 (PostGameplayEffectExecute에서 Armor/Health로 분배) */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	FGameplayAttributeData Damage = 0.f;
	ATTRIBUTE_ACCESSORS(UA1AttributeSet, Damage)

};