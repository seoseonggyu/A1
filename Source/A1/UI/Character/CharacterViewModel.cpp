// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Character/CharacterViewModel.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/A1VitalSet.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(CharacterViewModel)

DEFINE_LOG_CATEGORY(CharacterViewModelLog);

const FName UCharacterViewModel::ViewModelName = TEXT("CharacterViewModel");

UCharacterViewModel::UCharacterViewModel()
{
}

void UCharacterViewModel::InitializeViewModel(UObject* SourceObject)
{
	UAbilitySystemComponent* InASC = Cast<UAbilitySystemComponent>(SourceObject);
	if (!InASC)
	{
		UE_LOG(CharacterViewModelLog, Warning, TEXT("InitializeViewModel 실패: SourceObject가 ASC가 아닙니다"));
		return;
	}

	ASC = InASC;

	// Attribute 변경 델리게이트 바인딩
	InASC->GetGameplayAttributeValueChangeDelegate(UA1VitalSet::GetHealthAttribute()).AddUObject(this, &ThisClass::OnHealthChanged);
	InASC->GetGameplayAttributeValueChangeDelegate(UA1VitalSet::GetMaxHealthAttribute()).AddUObject(this, &ThisClass::OnMaxHealthChanged);
	InASC->GetGameplayAttributeValueChangeDelegate(UA1VitalSet::GetManaAttribute()).AddUObject(this, &ThisClass::OnManaChanged);
	InASC->GetGameplayAttributeValueChangeDelegate(UA1VitalSet::GetMaxManaAttribute()).AddUObject(this, &ThisClass::OnMaxManaChanged);
	InASC->GetGameplayAttributeValueChangeDelegate(UA1VitalSet::GetStaminaAttribute()).AddUObject(this, &ThisClass::OnStaminaChanged);
	InASC->GetGameplayAttributeValueChangeDelegate(UA1VitalSet::GetMaxStaminaAttribute()).AddUObject(this, &ThisClass::OnMaxStaminaChanged);

	// 초기 데이터 설정
	UpdateAllData();

	Super::InitializeViewModel(SourceObject);
}

void UCharacterViewModel::UninitializeViewModel()
{
	if (UAbilitySystemComponent* ASCPtr = ASC.Get())
	{
		ASCPtr->GetGameplayAttributeValueChangeDelegate(UA1VitalSet::GetHealthAttribute()).RemoveAll(this);
		ASCPtr->GetGameplayAttributeValueChangeDelegate(UA1VitalSet::GetMaxHealthAttribute()).RemoveAll(this);
		ASCPtr->GetGameplayAttributeValueChangeDelegate(UA1VitalSet::GetManaAttribute()).RemoveAll(this);
		ASCPtr->GetGameplayAttributeValueChangeDelegate(UA1VitalSet::GetMaxManaAttribute()).RemoveAll(this);
		ASCPtr->GetGameplayAttributeValueChangeDelegate(UA1VitalSet::GetStaminaAttribute()).RemoveAll(this);
		ASCPtr->GetGameplayAttributeValueChangeDelegate(UA1VitalSet::GetMaxStaminaAttribute()).RemoveAll(this);
	}

	ASC.Reset();

	Super::UninitializeViewModel();
}

void UCharacterViewModel::UpdateHealthData()
{
	UAbilitySystemComponent* ASCPtr = ASC.Get();
	if (!ASCPtr)
	{
		return;
	}

	bool bFound = false;
	const float CurrentHealth = ASCPtr->GetGameplayAttributeValue(UA1VitalSet::GetHealthAttribute(), bFound);
	const float CurrentMaxHealth = ASCPtr->GetGameplayAttributeValue(UA1VitalSet::GetMaxHealthAttribute(), bFound);

	UE_MVVM_SET_PROPERTY_VALUE(Health, CurrentHealth);
	UE_MVVM_SET_PROPERTY_VALUE(MaxHealth, CurrentMaxHealth);
	UE_MVVM_SET_PROPERTY_VALUE(HealthPercent, CurrentMaxHealth > 0.f ? CurrentHealth / CurrentMaxHealth : 0.f);
}

void UCharacterViewModel::UpdateManaData()
{
	UAbilitySystemComponent* ASCPtr = ASC.Get();
	if (!ASCPtr)
	{
		return;
	}

	bool bFound = false;
	const float CurrentMana = ASCPtr->GetGameplayAttributeValue(UA1VitalSet::GetManaAttribute(), bFound);
	const float CurrentMaxMana = ASCPtr->GetGameplayAttributeValue(UA1VitalSet::GetMaxManaAttribute(), bFound);

	UE_MVVM_SET_PROPERTY_VALUE(Mana, CurrentMana);
	UE_MVVM_SET_PROPERTY_VALUE(MaxMana, CurrentMaxMana);
	UE_MVVM_SET_PROPERTY_VALUE(ManaPercent, CurrentMaxMana > 0.f ? CurrentMana / CurrentMaxMana : 0.f);
}

void UCharacterViewModel::UpdateStaminaData()
{
	UAbilitySystemComponent* ASCPtr = ASC.Get();
	if (!ASCPtr)
	{
		return;
	}

	bool bFound = false;
	const float CurrentStamina = ASCPtr->GetGameplayAttributeValue(UA1VitalSet::GetStaminaAttribute(), bFound);
	const float CurrentMaxStamina = ASCPtr->GetGameplayAttributeValue(UA1VitalSet::GetMaxStaminaAttribute(), bFound);

	UE_MVVM_SET_PROPERTY_VALUE(Stamina, CurrentStamina);
	UE_MVVM_SET_PROPERTY_VALUE(MaxStamina, CurrentMaxStamina);
	UE_MVVM_SET_PROPERTY_VALUE(StaminaPercent, CurrentMaxStamina > 0.f ? CurrentStamina / CurrentMaxStamina : 0.f);
}


void UCharacterViewModel::UpdateAllData()
{
	UpdateHealthData();
	UpdateManaData();
	UpdateStaminaData();
}

void UCharacterViewModel::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	UpdateHealthData();
}

void UCharacterViewModel::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	UpdateHealthData();
}

void UCharacterViewModel::OnManaChanged(const FOnAttributeChangeData& Data)
{
	UpdateManaData();
}

void UCharacterViewModel::OnMaxManaChanged(const FOnAttributeChangeData& Data)
{
	UpdateManaData();
}

void UCharacterViewModel::OnStaminaChanged(const FOnAttributeChangeData& Data)
{
	UpdateStaminaData();
}

void UCharacterViewModel::OnMaxStaminaChanged(const FOnAttributeChangeData& Data)
{
	UpdateStaminaData();
}
