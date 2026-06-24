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

	UPDATE_ATTRIBUTE_GROUP(Health, MaxHealth, HealthPercent, UA1VitalSet::GetHealthAttribute(), UA1VitalSet::GetMaxHealthAttribute());
}

void UCharacterViewModel::UpdateManaData()
{
	UAbilitySystemComponent* ASCPtr = ASC.Get();
	if (!ASCPtr)
	{
		return;
	}
	UPDATE_ATTRIBUTE_GROUP(Mana, MaxMana, ManaPercent, UA1VitalSet::GetManaAttribute(), UA1VitalSet::GetMaxManaAttribute());
}

void UCharacterViewModel::UpdateStaminaData()
{
	UAbilitySystemComponent* ASCPtr = ASC.Get();
	if (!ASCPtr)
	{
		return;
	}

	UPDATE_ATTRIBUTE_GROUP(Stamina, MaxStamina, StaminaPercent, UA1VitalSet::GetStaminaAttribute(), UA1VitalSet::GetMaxStaminaAttribute());
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
