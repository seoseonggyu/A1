#include "A1VitalSet.h"
// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/A1VitalSet.h"
#include "A1GameplayTags.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Layout/CommonPrimaryGameLayout.h"
#include "UI/Character/CharacterViewModel.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1VitalSet)

DEFINE_LOG_CATEGORY(A1VitalSetLog);

UA1VitalSet::UA1VitalSet()
{
}

void UA1VitalSet::BeginDestroy()
{
	// 캐시된 ViewModel 정리
	if (IsValid(CachedViewModel))
	{
		CachedViewModel->UninitializeViewModel();
		CachedViewModel = nullptr;
	}

	Super::BeginDestroy();
}

void UA1VitalSet::PostNetInit()
{
	Super::PostNetInit();

	// 로컬 플레이어만 ViewModel 초기화
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(A1VitalSetLog, Warning, TEXT("[A1VitalSet] PostNetInit 실패: ASC를 찾을 수 없습니다"));
		return;
	}

	APawn* Pawn = Cast<APawn>(ASC->GetAvatarActor());
	if (!Pawn)
	{
		UE_LOG(A1VitalSetLog, Warning, TEXT("[A1VitalSet] PostNetInit 실패: AvatarActor가 Pawn이 아닙니다"));
		return;
	}

	const APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	// PrimaryLayout에서 ViewModel 가져와서 초기화
	UCommonPrimaryGameLayout* Layout = UCommonPrimaryGameLayout::GetPrimaryGameLayout(PC->GetLocalPlayer());
	if (!Layout)
	{
		UE_LOG(A1VitalSetLog, Warning, TEXT("[A1VitalSet] PostNetInit 실패: PrimaryGameLayout을 찾을 수 없습니다"));
		return;
	}

	CachedViewModel = Layout->GetViewModel<UCharacterViewModel>(UCharacterViewModel::ViewModelName);
	if (!CachedViewModel)
	{
		UE_LOG(A1VitalSetLog, Warning, TEXT("[A1VitalSet] PostNetInit 실패: CharacterViewModel을 생성할 수 없습니다"));
		return;
	}

	CachedViewModel->InitializeViewModel(ASC);
}

void UA1VitalSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UA1VitalSet, Health, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UA1VitalSet, MaxHealth, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UA1VitalSet, Mana, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UA1VitalSet, MaxMana, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UA1VitalSet, Stamina, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UA1VitalSet, MaxStamina, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UA1VitalSet, MoveSpeedMultiplier, Params);
}

void UA1VitalSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// TODO: 각 Attribute에 맞게
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

			// 서버에서만 Cue를 실행한다. 클라 복제는 ExecuteGameplayCue 내부의 멀티캐스트가 처리한다
			// (GameplayEvent_Death와 동일하게 IsOwnerActorAuthoritative로 가드).
			UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
			if (ASC && ASC->IsOwnerActorAuthoritative())
			{
				FGameplayCueParameters CueParams;
				CueParams.RawMagnitude = IncomingDamage;
				CueParams.EffectContext = Data.EffectSpec.GetContext();
				ASC->ExecuteGameplayCue(A1GameplayTags::GameplayCue_Character_DamageTaken, CueParams);
				ASC->ExecuteGameplayCue(A1GameplayTags::GameplayCue_Character_HitShake, CueParams);
			}
		}
	}
}

void UA1VitalSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UA1VitalSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UA1VitalSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	// TODO: 각 Attribute에 맞게
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());
	}
	else if (Attribute == GetMoveSpeedMultiplierAttribute())
	{
		// 슬로우가 여러 겹 겹쳐도 이동이 완전히 멈추거나 역방향으로 뒤집히지 않도록 하한을 둔다.
		NewValue = FMath::Clamp(NewValue, 0.1f, 2.f);
	}
}

void UA1VitalSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	// TODO: 각 Attribute에 맞게
	if (Attribute == GetHealthAttribute() && NewValue <= 0.f && OldValue > 0.f)
	{
		UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();

		// 서버(권위)에서만 사망 이벤트를 보낸다. 실제 사망 처리는 이 이벤트로 트리거되는
		// UA1Ability_Death가 전담한다 (Ability 기반 사망 처리).
		if (ASC && ASC->IsOwnerActorAuthoritative())
		{
			FGameplayEventData Payload;
			Payload.EventTag = A1GameplayTags::GameplayEvent_Death;
			Payload.Target = ASC->GetAvatarActor();

			FScopedPredictionWindow NewScopedWindow(ASC, true);
			ASC->HandleGameplayEvent(A1GameplayTags::GameplayEvent_Death, &Payload);
		}
	}
}

void UA1VitalSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UA1VitalSet, Health, OldValue);
}

void UA1VitalSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UA1VitalSet, MaxHealth, OldValue);
}

void UA1VitalSet::OnRep_Mana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UA1VitalSet, Mana, OldValue);
}

void UA1VitalSet::OnRep_MaxMana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UA1VitalSet, MaxMana, OldValue);
}

void UA1VitalSet::OnRep_Stamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UA1VitalSet, Stamina, OldValue);
}

void UA1VitalSet::OnRep_MaxStamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UA1VitalSet, MaxStamina, OldValue);
}

void UA1VitalSet::OnRep_MoveSpeedMultiplier(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UA1VitalSet, MoveSpeedMultiplier, OldValue);
}
