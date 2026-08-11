// Copyright Epic Games, Inc. All Rights Reserved.

#include "A1Ability_Interact.h"

#include "A1GameplayTags.h"
#include "AbilitySystemComponent.h"
#include "Interaction/A1Interactable.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_Interact)

DEFINE_LOG_CATEGORY(A1AbilityInteractLog);

UA1Ability_Interact::UA1Ability_Interact(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 입력이 아니라 스캔 어빌리티(UA1Ability_Interact_Scan)가 보내는 GameplayEvent로 트리거된다.
	ActivationPolicy = ECommonAbilityActivationPolicy::Manual;

	SetAssetTags(FGameplayTagContainer(A1GameplayTags::Ability_Interact));
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_Interacting);

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = A1GameplayTags::GameplayEvent_Interact;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UA1Ability_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AActor* Interactor = GetAvatarActorFromActorInfo();

	// 스캔 어빌리티가 채워 보낸 대상. LocalPredicted라 서버에도 TriggerEventData가 함께 전달된다.
	AActor* TargetActor = (TriggerEventData != nullptr) ? const_cast<AActor*>(TriggerEventData->Target.Get()) : nullptr;
	IA1Interactable* Interactable = Cast<IA1Interactable>(TargetActor);

	// 실제 결과 처리는 서버(Authority)에서만 수행한다. (클라는 예측 활성화만 하고 결과는 서버가 권위적으로 처리)
	if (HasAuthority(&ActivationInfo) && Interactable != nullptr)
	{
		FA1InteractionQuery Query;
		Query.RequestingAvatar = Interactor;
		Query.RequestingController = GetControllerFromActorInfo();

		// 서버 재검증: 클라 예측 시점과 달라졌을 수 있으므로 다시 확인한다.
		if (Interactable->CanInteract(Query))
		{
			// 대상이 확장 이벤트 태그를 제공하면, 후속 어빌리티(줍기/문 열기 등)를 소유자 ASC에서 실행하도록 보낸다.
			TArray<FA1InteractionOption> Options;
			Interactable->GatherInteractionOptions(Query, Options);
			if (Options.Num() > 0 && Options[0].InteractEventTag.IsValid())
			{
				if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
				{
					FGameplayEventData Payload;
					Payload.EventTag = Options[0].InteractEventTag;
					Payload.Instigator = Interactor;
					Payload.Target = TargetActor;
					ASC->HandleGameplayEvent(Options[0].InteractEventTag, &Payload);
				}
			}

			// 기본 결과 처리.
			Interactable->OnInteractAuth(Interactor);

			UE_LOG(A1AbilityInteractLog, Log, TEXT("Interact 실행: Interactor=%s Target=%s"), *GetNameSafe(Interactor), *GetNameSafe(TargetActor));
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
