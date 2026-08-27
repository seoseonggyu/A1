// Copyright Epic Games, Inc. All Rights Reserved.

#include "A1Ability_Interact_Hold.h"

#include "A1GameplayTags.h"
#include "CommonActivatableWidget.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Coro.h"
#include "Awaiters/Time.h"
#include "CommonUIExtensionTags.h"
#include "GameFramework/PlayerController.h"
#include "Interaction/A1Interactable.h"
#include "Layout/CommonPrimaryGameLayout.h"
#include "UI/Interaction/A1InteractionHoldWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_Interact_Hold)

DEFINE_LOG_CATEGORY(A1AbilityInteractHoldLog);

UA1Ability_Interact_Hold::UA1Ability_Interact_Hold(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 대상의 InteractEventTag로 서버에서만 트리거된다. 실제 트리거 태그는 파생 클래스가 지정한다.
	ActivationPolicy = ECommonAbilityActivationPolicy::Manual;

	// 서버가 로컬로 발동을 결정하고, 그 활성화가 소유 클라에 복제되어 각자 자기 쪽에서 홀드를 진행한다.
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	ActivationOwnedTags.AddTag(A1GameplayTags::Status_Interacting);
}

void UA1Ability_Interact_Hold::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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
	// 몽타주 자체의 루프 섹션으로 반복되며, 어빌리티가 끝나면 자동으로(bStopWhenAbilityEnds) 멈춘다.
	if (HoldMontage != nullptr)
	{
		if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("InteractHold"), HoldMontage))
		{
			PlayMontageTask->ReadyForActivation();
		}
	}

	// 홀드 진행 UI는 순수 로컬 연출이므로 소유 클라에서만 띄운다.
	if (HasAuthority(&ActivationInfo) == false)
	{
		ShowHoldWidgetLocal();
	}

	PendingHoldTask = RunHoldCoroutine();
}

void UA1Ability_Interact_Hold::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (bHoldWidgetVisible)
	{
		HideHoldWidgetLocal();
	}

	TargetActor = nullptr;
	bInputReleased = false;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

TCoroTask<void> UA1Ability_Interact_Hold::RunHoldCoroutine()
{
	co_await Coro::Latent::Seconds(this, HoldDuration);

	// 대기 중 입력이 풀렸거나(OnInputReleased가 CancelAbility로 이미 종료) 다른 이유로 종료됐으면 조용히 끝낸다.
	if (bInputReleased || IsActive() == false)
	{
		co_return;
	}

	AActor* Interactor = GetAvatarActorFromActorInfo();
	AActor* Target = TargetActor.Get();

	if (HasAuthority(&CurrentActivationInfo))
	{
		OnHoldCompletedAuth(Interactor, Target);
		UE_LOG(A1AbilityInteractHoldLog, Log, TEXT("홀드 완료(Auth): Interactor=%s Target=%s"), *GetNameSafe(Interactor), *GetNameSafe(Target));
	}
	else
	{
		OnHoldCompletedLocal(Interactor, Target);
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UA1Ability_Interact_Hold::OnInputReleased(float TimeHeld)
{
	bInputReleased = true;

	if (IsActive())
	{
		UE_LOG(A1AbilityInteractHoldLog, Verbose, TEXT("홀드 중단: TimeHeld=%.2f"), TimeHeld);
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}

void UA1Ability_Interact_Hold::ShowHoldWidgetLocal()
{
	if (UA1InteractionHoldWidget* Widget = FindHoldWidgetLocal())
	{
		Widget->StartHold(HoldDuration);
		bHoldWidgetVisible = true;
	}
	else
	{
		UE_LOG(A1AbilityInteractHoldLog, Warning, TEXT("ShowHoldWidgetLocal: HUD에서 UA1InteractionHoldWidget을 찾을 수 없습니다"));
	}
}

void UA1Ability_Interact_Hold::HideHoldWidgetLocal()
{
	bHoldWidgetVisible = false;

	if (UA1InteractionHoldWidget* Widget = FindHoldWidgetLocal())
	{
		Widget->StopHold();
	}
}

UA1InteractionHoldWidget* UA1Ability_Interact_Hold::FindHoldWidgetLocal() const
{
	APlayerController* PC = Cast<APlayerController>(GetControllerFromActorInfo());
	UCommonPrimaryGameLayout* Layout = PC ? UCommonPrimaryGameLayout::GetPrimaryGameLayout(PC->GetLocalPlayer()) : nullptr;
	if (Layout == nullptr)
	{
		return nullptr;
	}

	// 홀드 UI는 Push/Pop 없이, UI.Layer.Game에 Push된 위젯(W_GameLayout)의 자식으로 상시 배치돼
	// 있다. Layout 자신의 WidgetTree에는 그 런타임 자식이 없으므로, Game 레이어의 "현재 활성
	// 위젯"을 먼저 찾고 그 안에서 검색해야 한다.
	UCommonActivatableWidgetContainerBase* GameLayer = Layout->GetLayerContainer(CommonUIExtensionTags::UI_Layer_Game);
	UCommonActivatableWidget* ActiveGameWidget = GameLayer ? GameLayer->GetActiveWidget() : nullptr;
	if (ActiveGameWidget == nullptr)
	{
		return nullptr;
	}

	return Layout->FindWidgetOfType<UA1InteractionHoldWidget>(ActiveGameWidget);
}
