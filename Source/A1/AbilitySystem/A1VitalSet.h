// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/CommonAttributeSet.h"
#include "A1VitalSet.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1VitalSetLog, Log, All);

/**
 * 캐릭터 기본 스탯 AttributeSet
 *
 */
UCLASS()
class A1_API UA1VitalSet : public UCommonAttributeSet
{
	GENERATED_BODY()

public:
	UA1VitalSet();

	virtual void BeginDestroy() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	virtual void PostNetInit() override;

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);


	/** 초기화된 ViewModel 캐시 (BeginDestroy에서 정리용) */
	UPROPERTY()
	TObjectPtr<class UCharacterViewModel> CachedViewModel;

public:

	// TODO: 여기서 이 값으로 설정하는 게 아니라 Effect로
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health = 100.f;
	ATTRIBUTE_ACCESSORS(UA1VitalSet, Health)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_MaxHealth, meta = (ShowOnlyInnerProperties))
	FGameplayAttributeData MaxHealth = 100.f;
	ATTRIBUTE_ACCESSORS(UA1VitalSet, MaxHealth)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mana", ReplicatedUsing = OnRep_Mana)
	FGameplayAttributeData Mana = 100.f;
	ATTRIBUTE_ACCESSORS(UA1VitalSet, Mana)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mana", ReplicatedUsing = OnRep_MaxMana, meta = (ShowOnlyInnerProperties))
	FGameplayAttributeData MaxMana = 100.f;
	ATTRIBUTE_ACCESSORS(UA1VitalSet, MaxMana)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina = 150.f;
	ATTRIBUTE_ACCESSORS(UA1VitalSet, Stamina)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", ReplicatedUsing = OnRep_MaxStamina, meta = (ShowOnlyInnerProperties))
	FGameplayAttributeData MaxStamina = 150.f;
	ATTRIBUTE_ACCESSORS(UA1VitalSet, MaxStamina)
	

	//-----------------------------------------------------------------------------
	// Meta Attributes (복제되지 않음, 계산용)
	//-----------------------------------------------------------------------------
		
	/** 들어온 데미지 (PostGameplayEffectExecute에서 Armor/Health로 분배) */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	FGameplayAttributeData Damage = 0.f;
	ATTRIBUTE_ACCESSORS(UA1VitalSet, Damage)

};