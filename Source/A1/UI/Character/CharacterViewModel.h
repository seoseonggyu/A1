// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ViewModel/CommonViewModelBase.h"
#include "CharacterViewModel.generated.h"

class UAbilitySystemComponent;
struct FOnAttributeChangeData;
struct FGameplayAttribute;

#define UPDATE_ATTRIBUTE_GROUP( \
	CurrentProp, \
	MaxProp, \
	PercentProp, \
	CurrentAttr, \
	MaxAttr) \
{ \
	bool bFound = false; \
	const float CurrentValue = ASCPtr->GetGameplayAttributeValue(CurrentAttr, bFound); \
	const float MaxValue = ASCPtr->GetGameplayAttributeValue(MaxAttr, bFound); \
	UE_MVVM_SET_PROPERTY_VALUE(CurrentProp, CurrentValue); \
	UE_MVVM_SET_PROPERTY_VALUE(MaxProp, MaxValue); \
	UE_MVVM_SET_PROPERTY_VALUE(PercentProp, MaxValue > 0.f ? CurrentValue / MaxValue : 0.f); \
}


DECLARE_LOG_CATEGORY_EXTERN(CharacterViewModelLog, Log, All);

/**
 * 캐릭터 스탯 ViewModel
 *
 * 캐릭터의 Health, Mana 등 스탯 데이터를 UI에 노출합니다.
 * ASC의 Attribute 변경 델리게이트를 통해 자동으로 업데이트됩니다.
 */
UCLASS(BlueprintType)
class A1_API UCharacterViewModel : public UCommonViewModelBase
{
	GENERATED_BODY()

public:
	UCharacterViewModel();

	virtual void InitializeViewModel(UObject* SourceObject) override;
	virtual void UninitializeViewModel() override;


	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Character ViewModel|Health")
	float Health = 100.f;
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Character ViewModel|Health")
	float MaxHealth = 100.f;
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Character ViewModel|Health")
	float HealthPercent = 1.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Character ViewModel|Mana")
	float Mana = 100.f;
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Character ViewModel|Mana")
	float MaxMana = 100.f;
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Character ViewModel|Mana")
	float ManaPercent = 1.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Character ViewModel|Stamina")
	float Stamina = 100.f;
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Character ViewModel|Stamina")
	float MaxStamina = 100.f;
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Character ViewModel|Stamina")
	float StaminaPercent = 1.f;

	void UpdateHealthData();
	void UpdateManaData();
	void UpdateStaminaData();
	void UpdateAllData();

public:
	/** ViewModel 이름 (MVVM 바인딩에서 사용) */
	static const FName ViewModelName;

protected:
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> ASC;

private:
	void OnHealthChanged(const FOnAttributeChangeData& Data);
	void OnMaxHealthChanged(const FOnAttributeChangeData& Data);

	void OnManaChanged(const FOnAttributeChangeData& Data);
	void OnMaxManaChanged(const FOnAttributeChangeData& Data);

	void OnStaminaChanged(const FOnAttributeChangeData& Data);
	void OnMaxStaminaChanged(const FOnAttributeChangeData& Data);
};