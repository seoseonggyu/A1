// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Character/CharacterViewModel.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/A1AttributeSet.h"
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
	InASC->GetGameplayAttributeValueChangeDelegate(UA1AttributeSet::GetHealthAttribute()).AddUObject(this, &ThisClass::OnHealthChanged);
	InASC->GetGameplayAttributeValueChangeDelegate(UA1AttributeSet::GetMaxHealthAttribute()).AddUObject(this, &ThisClass::OnMaxHealthChanged);

	// 초기 데이터 설정
	UpdateAllData();

	Super::InitializeViewModel(SourceObject);
}

void UCharacterViewModel::UninitializeViewModel()
{
	if (UAbilitySystemComponent* ASCPtr = ASC.Get())
	{
		ASCPtr->GetGameplayAttributeValueChangeDelegate(UA1AttributeSet::GetHealthAttribute()).RemoveAll(this);
		ASCPtr->GetGameplayAttributeValueChangeDelegate(UA1AttributeSet::GetMaxHealthAttribute()).RemoveAll(this);
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
	const float CurrentHealth = ASCPtr->GetGameplayAttributeValue(UA1AttributeSet::GetHealthAttribute(), bFound);
	const float CurrentMaxHealth = ASCPtr->GetGameplayAttributeValue(UA1AttributeSet::GetMaxHealthAttribute(), bFound);

	UE_MVVM_SET_PROPERTY_VALUE(Health, CurrentHealth);
	UE_MVVM_SET_PROPERTY_VALUE(MaxHealth, CurrentMaxHealth);
	UE_MVVM_SET_PROPERTY_VALUE(HealthPercent, CurrentMaxHealth > 0.f ? CurrentHealth / CurrentMaxHealth : 0.f);
}


void UCharacterViewModel::UpdateAllData()
{
	UpdateHealthData();
}

void UCharacterViewModel::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	UpdateHealthData();
}

void UCharacterViewModel::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	UpdateHealthData();
}