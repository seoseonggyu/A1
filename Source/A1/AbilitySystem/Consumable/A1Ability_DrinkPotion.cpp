// Copyright Epic Games, Inc. All Rights Reserved.

#include "A1Ability_DrinkPotion.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Equipment/EquipmentInstance.h"
#include "Equipment/Fragment/EquipmentFragment_Consume.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_DrinkPotion)

DEFINE_LOG_CATEGORY(A1Ability_DrinkPotionLog);

UA1Ability_DrinkPotion::UA1Ability_DrinkPotion(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UA1Ability_DrinkPotion::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	bItemUsed = false;

	// 손에 든 포션 + 회복 GE가 유효할 때만 진행한다. (없으면 소비/회복 없이 취소)
	const UEquipmentInstance* Consumable = GetConsumableEquipmentInstance();
	const FEquipmentFragment_Consume* ConsumeFragment = Consumable ? Consumable->FindFragment<FEquipmentFragment_Consume>() : nullptr;
	if (ConsumeFragment == nullptr || ConsumeFragment->ConsumeEffectClass == nullptr)
	{
		UE_LOG(A1Ability_DrinkPotionLog, Warning, TEXT("DrinkPotion: 소비형 장비 또는 ConsumeEffectClass가 없어 취소합니다."));
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	// 몽타주가 없으면 즉시 완료 처리한다.
	if (DrinkMontage == nullptr)
	{
		OnDrinkCompleted();
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("DrinkPotion"), DrinkMontage, 1.f, NAME_None, true);
	if (MontageTask == nullptr)
	{
		OnDrinkCompleted();
		return;
	}
	
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnDrinkCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnDrinkCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnDrinkCancelled);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnDrinkCancelled);
	MontageTask->ReadyForActivation();
}

void UA1Ability_DrinkPotion::OnDrinkCompleted()
{
	// 회복 GE 적용과 아이템 소비는 서버 권한에서만 실제 반영된다. (Attribute·인벤토리는 복제로 클라 동기화)
	ApplyConsumeEffect();
	ConsumeSourceItemAuth();

	bItemUsed = true;

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UA1Ability_DrinkPotion::OnDrinkCancelled()
{
	// 마시기가 중단되면 회복·소비 없이 취소로 종료한다.
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UA1Ability_DrinkPotion::ApplyConsumeEffect()
{
	if (HasAuthority(&CurrentActivationInfo) == false)
	{
		return;
	}

	const UEquipmentInstance* Consumable = GetConsumableEquipmentInstance();
	if (Consumable == nullptr)
	{
		return;
	}

	const FEquipmentFragment_Consume* ConsumeFragment = Consumable->FindFragment<FEquipmentFragment_Consume>();
	if (ConsumeFragment == nullptr || ConsumeFragment->ConsumeEffectClass == nullptr)
	{
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(ConsumeFragment->ConsumeEffectClass);
	if (SpecHandle.IsValid() == false)
	{
		return;
	}

	// 포션이 지정한 회복 GE(예: Health/Mana Additive)를 소유자에게 적용한다.
	(void)ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle);
}
