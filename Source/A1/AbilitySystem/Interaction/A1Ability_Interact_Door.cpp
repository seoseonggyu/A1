// Copyright Epic Games, Inc. All Rights Reserved.

#include "A1Ability_Interact_Door.h"

#include "A1GameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Coro.h"
#include "Awaiters/Time.h"
#include "Interaction/A1Interactable.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_Interact_Door)

DEFINE_LOG_CATEGORY(A1AbilityInteractDoorLog);

UA1Ability_Interact_Door::UA1Ability_Interact_Door(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// UA1Ability_Interact가 서버에서만 보내는 GameplayEvent로 트리거된다.
	ActivationPolicy = ECommonAbilityActivationPolicy::Manual;

	// 서버가 로컬로 발동을 결정하고, 그 활성화가 소유 클라에 복제되어 각자 자기 쪽에서 홀드를 진행한다.
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	SetAssetTags(FGameplayTagContainer(A1GameplayTags::Ability_Interact_Door));
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_Interacting);

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = A1GameplayTags::GameplayEvent_Interact_Door;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UA1Ability_Interact_Door::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AActor* Target = (TriggerEventData != nullptr) ? const_cast<AActor*>(TriggerEventData->Target.Get()) : nullptr;
	if (Cast<IA1Interactable>(Target) == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	TargetActor = Target;
	bInputReleased = false;

	// 홀드 도중 상호작용 입력이 풀리면 OnInputReleased가 호출된다.
	// Input.Ability.Interact로 부여되어 있어야(헤더 주석 참고) 릴리즈 신호를 받을 수 있다.
	if (UAbilityTask_WaitInputRelease* ReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true))
	{
		ReleaseTask->OnRelease.AddDynamic(this, &ThisClass::OnInputReleased);
		ReleaseTask->ReadyForActivation();
	}

	// 홀드 중 재생할 연출(선택). 완료 판정은 코루틴이 하므로 이 몽타주는 결과와 무관한 순수 연출이다.
	if (HoldMontage != nullptr)
	{
		if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("InteractDoorHold"), HoldMontage))
		{
			PlayMontageTask->ReadyForActivation();
		}
	}

	PendingHoldTask = RunHoldCoroutine();
}

void UA1Ability_Interact_Door::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	TargetActor = nullptr;
	bInputReleased = false;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

TCoroTask<void> UA1Ability_Interact_Door::RunHoldCoroutine()
{
	co_await Coro::Latent::Seconds(this, HoldDuration);

	// 대기 중 입력이 풀렸거나(OnInputReleased가 CancelAbility로 이미 종료) 다른 이유로 종료됐으면 조용히 끝낸다.
	if (bInputReleased || IsActive() == false)
	{
		co_return;
	}

	AActor* Interactor = GetAvatarActorFromActorInfo();
	IA1Interactable* Interactable = Cast<IA1Interactable>(TargetActor.Get());

	// 홀드를 완주한 서버 인스턴스에서만 결과를 처리한다.
	if (HasAuthority(&CurrentActivationInfo) && Interactable != nullptr)
	{
		Interactable->OnInteractAuth(Interactor);
		UE_LOG(A1AbilityInteractDoorLog, Log, TEXT("Door 홀드 완료: Interactor=%s Target=%s"), *GetNameSafe(Interactor), *GetNameSafe(TargetActor.Get()));
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UA1Ability_Interact_Door::OnInputReleased(float TimeHeld)
{
	bInputReleased = true;

	if (IsActive())
	{
		UE_LOG(A1AbilityInteractDoorLog, Verbose, TEXT("Door 홀드 중단: TimeHeld=%.2f"), TimeHeld);
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}
