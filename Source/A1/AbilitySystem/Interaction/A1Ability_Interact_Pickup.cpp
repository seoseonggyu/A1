// Copyright Epic Games, Inc. All Rights Reserved.

#include "A1Ability_Interact_Pickup.h"

#include "A1GameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Interaction/A1Interactable.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_Interact_Pickup)

DEFINE_LOG_CATEGORY(A1AbilityInteractPickupLog);

UA1Ability_Interact_Pickup::UA1Ability_Interact_Pickup(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// UA1Ability_Interact가 서버에서만 보내는 GameplayEvent로 트리거된다.
	ActivationPolicy = ECommonAbilityActivationPolicy::Manual;

	// 서버가 로컬로 발동을 결정하고, 그 활성화가 소유 클라에 복제되어 각자 자기 쪽 몽타주를 재생한다.
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	SetAssetTags(FGameplayTagContainer(A1GameplayTags::Ability_Interact_Pickup));
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_Interacting);

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = A1GameplayTags::GameplayEvent_Interact_Pickup;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UA1Ability_Interact_Pickup::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AActor* Interactor = GetAvatarActorFromActorInfo();
	AActor* TargetActor = (TriggerEventData != nullptr) ? const_cast<AActor*>(TriggerEventData->Target.Get()) : nullptr;
	IA1Interactable* Interactable = Cast<IA1Interactable>(TargetActor);

	// 결과 처리(획득/제거)는 서버 인스턴스에서만 즉시 수행한다. 몽타주는 순수 연출이라 결과 처리를 기다리지 않는다.
	if (HasAuthority(&ActivationInfo) && Interactable != nullptr)
	{
		Interactable->OnInteractAuth(Interactor);
		UE_LOG(A1AbilityInteractPickupLog, Log, TEXT("Pickup 처리: Interactor=%s Target=%s"), *GetNameSafe(Interactor), *GetNameSafe(TargetActor));
	}

	if (PickupMontage != nullptr)
	{
		if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("InteractPickup"), PickupMontage))
		{
			PlayMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageEnded);
			PlayMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageEnded);
			PlayMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageEnded);
			PlayMontageTask->ReadyForActivation();
			return;
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UA1Ability_Interact_Pickup::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
