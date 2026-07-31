// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/CommonAttributeSet.h"
#include "A1VitalSet.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1VitalSetLog, Log, All);


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
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	virtual void PostNetInit() override;

protected:
	/**
	 * Health/Mana/Stamina를 [0, Max]로 클램프한다.
	 * PreAttributeChange(CurrentValue용)와 PreAttributeBaseChange(BaseValue용) 양쪽에서 공유해서 호출해야 한다.
	 * BaseValue 쪽을 누락하면, 이미 Max인 상태에서도 Periodic/Instant GE가 BaseValue를 Max 이상으로 계속 쌓을 수 있고
	 * (CurrentValue만 Max로 눌려 보이므로 겉보기엔 안 줄어드는 것처럼 보이다가, 몇 차례 소모돼 BaseValue가
	 * Max 아래로 내려가야 비로소 화면에 반영되는 버그가 생긴다.
	 */
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

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

	UPROPERTY()
	TObjectPtr<class UCharacterViewModel> CachedViewModel;

public:

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
	// Meta Attributes (�������� ����, ����)
	//-----------------------------------------------------------------------------
		
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	FGameplayAttributeData Damage = 0.f;
	ATTRIBUTE_ACCESSORS(UA1VitalSet, Damage)

};