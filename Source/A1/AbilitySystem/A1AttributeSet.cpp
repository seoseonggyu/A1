#include "A1AttributeSet.h"
// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/A1AttributeSet.h"
#include "Player/A1Character.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Layout/CommonPrimaryGameLayout.h"
#include "UI/Character/CharacterViewModel.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1AttributeSet)

DEFINE_LOG_CATEGORY(A1AttributeSetLog);

UA1AttributeSet::UA1AttributeSet()
{
}

void UA1AttributeSet::BeginDestroy()
{
	// 캐시된 ViewModel 정리
	if (IsValid(CachedViewModel))
	{
		CachedViewModel->UninitializeViewModel();
		CachedViewModel = nullptr;
	}

	Super::BeginDestroy();
}

void UA1AttributeSet::PostNetInit()
{
	Super::PostNetInit();

	// 로컬 플레이어만 ViewModel 초기화
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(A1AttributeSetLog, Warning, TEXT("[A1AttributeSet] PostNetInit 실패: ASC를 찾을 수 없습니다"));
		return;
	}

	APawn* Pawn = Cast<APawn>(ASC->GetAvatarActor());
	if (!Pawn)
	{
		UE_LOG(A1AttributeSetLog, Warning, TEXT("[A1AttributeSet] PostNetInit 실패: AvatarActor가 Pawn이 아닙니다"));
		return;
	}

	const APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC || !PC->IsLocalController())
	{
		// 로컬 플레이어가 아니면 정상적으로 스킵
		return;
	}

	// PrimaryLayout에서 ViewModel 가져와서 초기화
	UCommonPrimaryGameLayout* Layout = UCommonPrimaryGameLayout::GetPrimaryGameLayout(PC->GetLocalPlayer());
	if (!Layout)
	{
		UE_LOG(A1AttributeSetLog, Warning, TEXT("[A1AttributeSet] PostNetInit 실패: PrimaryGameLayout을 찾을 수 없습니다"));
		return;
	}

	CachedViewModel = Layout->GetViewModel<UCharacterViewModel>(UCharacterViewModel::ViewModelName);
	if (!CachedViewModel)
	{
		UE_LOG(A1AttributeSetLog, Warning, TEXT("[A1AttributeSet] PostNetInit 실패: CharacterViewModel을 생성할 수 없습니다"));
		return;
	}

	CachedViewModel->InitializeViewModel(ASC);
}

void UA1AttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// PushModel 사용
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UA1AttributeSet, Health, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UA1AttributeSet, MaxHealth, Params);
}

void UA1AttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// Damage Meta Attribute 처리
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		const float IncomingDamage = GetDamage();
		SetDamage(0.f);

		if (IncomingDamage > 0.f)
		{
			float RemainingDamage = IncomingDamage;
			// TODO: Armor
			// const float CurrentArmor = GetArmor(); 

			// Armor가 있으면 데미지 흡수 (A1 스타일: Armor가 50% 흡수)
			/*if (CurrentArmor > 0.f)
			{
				constexpr float ArmorAbsorptionRate = 0.5f;
				const float DamageToArmor = FMath::Min(IncomingDamage * ArmorAbsorptionRate, CurrentArmor);

				SetArmor(CurrentArmor - DamageToArmor);
				RemainingDamage = IncomingDamage - DamageToArmor;
			}*/

			// 남은 데미지를 Health에 적용
			if (RemainingDamage > 0.f)
			{
				const float NewHealth = FMath::Max(GetHealth() - RemainingDamage, 0.f);
				SetHealth(NewHealth);
			}
		}
	}
}

void UA1AttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Health를 0 ~ MaxHealth 범위로 클램프
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
}

void UA1AttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	// Health가 0이 되면 죽음 처리
	if (Attribute == GetHealthAttribute() && NewValue <= 0.f && OldValue > 0.f)
	{
		if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
		{
			if (AA1Character* Character = Cast<AA1Character>(ASC->GetAvatarActor()))
			{
				// Character->HandleDeathAuth(); // TODO: Death 처리
			}
		}
	}
}

void UA1AttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UA1AttributeSet, Health, OldValue);
}

void UA1AttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UA1AttributeSet, MaxHealth, OldValue);
}