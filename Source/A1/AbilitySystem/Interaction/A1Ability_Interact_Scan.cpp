// Copyright Epic Games, Inc. All Rights Reserved.

#include "A1Ability_Interact_Scan.h"

#include "A1GameplayTags.h"
#include "AbilitySystem/Tasks/A1AbilityTask_WaitInputStart.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Interaction/A1Interactable.h"
#include "Layout/CommonPrimaryGameLayout.h"
#include "Physics/A1CollisionChannels.h"
#include "TimerManager.h"
#include "UI/Interaction/InteractionPromptViewModel.h"

#if ENABLE_DRAW_DEBUG
#include "DrawDebugHelpers.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_Interact_Scan)

DEFINE_LOG_CATEGORY(A1AbilityInteractScanLog);

UA1Ability_Interact_Scan::UA1Ability_Interact_Scan(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 스폰(부여) 직후 자동 활성화.
	ActivationPolicy = ECommonAbilityActivationPolicy::OnSpawn;
	ActivationGroup = ECommonAbilityActivationGroup::Independent;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 클라이언트에서만 실행되며 서버로 활성화 RPC를 보내지 않는다.
	// 원격 클라의 OnSpawn 활성화는 UCommonAbilitySystemComponent::TryActivateLocalOnlyAbilitiesOnSpawn이 담당한다.
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;

	SetAssetTags(FGameplayTagContainer(A1GameplayTags::Ability_Interact_Scan));
}

bool UA1Ability_Interact_Scan::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	// 데디케이티드 서버 사본이나 원격 폰에서는 어떤 경로로도 활성화되지 않도록 한다. (로컬 컨트롤 폰만 허용)
	if (ActorInfo == nullptr || ActorInfo->IsLocallyControlled() == false)
	{
		return false;
	}

	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UA1Ability_Interact_Scan::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 스캔과 입력 감지는 순수 로컬 연출이므로 소유 클라(또는 호스트)에서만 수행한다.
	if (IsLocallyControlled() == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ScanTimerHandle, this, &ThisClass::ScanLocal, ScanRate, true, 0.0f);
	}

	WaitForInputStart();
}

void UA1Ability_Interact_Scan::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ScanTimerHandle);
	}

	// 남아있는 하이라이트 정리.
	SetCurrentTargetLocal(nullptr);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UA1Ability_Interact_Scan::ScanLocal()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	UWorld* World = GetWorld();
	if (Avatar == nullptr || World == nullptr)
	{
		SetCurrentTargetLocal(nullptr);
		return;
	}

	const FVector Center = Avatar->GetActorLocation();

	FCollisionQueryParams Params(SCENE_QUERY_STAT(A1Ability_Interact_Scan), false);
	Params.AddIgnoredActor(Avatar);

	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByChannel(Overlaps, Center, FQuat::Identity, A1_TraceChannel_Interaction, FCollisionShape::MakeSphere(ScanRange), Params);

	FA1InteractionQuery Query;
	Query.RequestingAvatar = Avatar;
	Query.RequestingController = GetControllerFromActorInfo();

	// IA1Interactable을 구현하고 현재 상호작용 가능한 대상 중 가장 가까운 것을 고른다.
	AActor* Best = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		IA1Interactable* Interactable = Cast<IA1Interactable>(HitActor);
		if (Interactable == nullptr || HitActor == Avatar)
		{
			continue;
		}

		if (Interactable->CanInteract(Query) == false)
		{
			continue;
		}

		const float DistSq = FVector::DistSquared2D(Center, HitActor->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = HitActor;
		}
	}

	UE_LOG(A1AbilityInteractScanLog, VeryVerbose, TEXT("ScanLocal: Overlaps=%d Best=%s"), Overlaps.Num(), *GetNameSafe(Best));

	SetCurrentTargetLocal(Best);

#if ENABLE_DRAW_DEBUG
	if (bShowScanDebug)
	{
		DrawDebugSphere(World, Center, ScanRange, 16, Best ? FColor::Green : FColor::Silver, false, ScanRate);
	}
#endif
}

void UA1Ability_Interact_Scan::SetCurrentTargetLocal(AActor* NewTarget)
{
	if (NewTarget == CurrentTarget.Get())
	{
		return;
	}

	// 이전 대상 하이라이트 해제 후 새 대상 하이라이트.
	SetInteractableHighlightLocal(CurrentTarget.Get(), false);
	CurrentTarget = NewTarget;
	SetInteractableHighlightLocal(NewTarget, true);

	// 하이라이트와 함께 상호작용 프롬프트("줍기" 등) UI도 갱신한다.
	UpdateInteractionPromptLocal(NewTarget);
}

void UA1Ability_Interact_Scan::SetInteractableHighlightLocal(AActor* InteractableActor, bool bHighlight) const
{
	IA1Interactable* Interactable = Cast<IA1Interactable>(InteractableActor);
	if (Interactable == nullptr)
	{
		return;
	}

	int32 Stencil = 1;
	if (bHighlight)
	{
		// 대상이 제공하는 스텐실 값으로 외곽선 색을 구분한다.
		FA1InteractionQuery Query;
		Query.RequestingAvatar = GetAvatarActorFromActorInfo();
		Query.RequestingController = GetControllerFromActorInfo();

		TArray<FA1InteractionOption> Options;
		Interactable->GatherInteractionOptions(Query, Options);
		if (Options.Num() > 0)
		{
			Stencil = Options[0].HighlightStencil;
		}
	}

	TArray<UPrimitiveComponent*> Components;
	Interactable->GetHighlightComponents(Components);
	for (UPrimitiveComponent* Component : Components)
	{
		if (Component == nullptr)
		{
			continue;
		}

		Component->SetRenderCustomDepth(bHighlight);
		if (bHighlight)
		{
			Component->SetCustomDepthStencilValue(Stencil);
		}
	}
}

void UA1Ability_Interact_Scan::UpdateInteractionPromptLocal(AActor* Target) const
{
	UInteractionPromptViewModel* PromptViewModel = GetPromptViewModelLocal();
	if (PromptViewModel == nullptr)
	{
		// 아직 HUD 위젯(ViewModel)이 준비되지 않았으면 다음 스캔에서 다시 시도한다.
		return;
	}

	IA1Interactable* Interactable = Cast<IA1Interactable>(Target);
	if (Interactable == nullptr)
	{
		PromptViewModel->HidePrompt();
		return;
	}

	// 대상이 제공하는 첫 옵션의 문구(Title)를 프롬프트로 표시한다. (예: "줍기")
	FA1InteractionQuery Query;
	Query.RequestingAvatar = GetAvatarActorFromActorInfo();
	Query.RequestingController = GetControllerFromActorInfo();

	TArray<FA1InteractionOption> Options;
	Interactable->GatherInteractionOptions(Query, Options);
	if (Options.Num() > 0)
	{
		PromptViewModel->ShowPrompt(Options[0].Title);
	}
	else
	{
		PromptViewModel->HidePrompt();
	}
}

UInteractionPromptViewModel* UA1Ability_Interact_Scan::GetPromptViewModelLocal() const
{
	if (CachedPromptViewModel.IsValid())
	{
		return CachedPromptViewModel.Get();
	}

	const APlayerController* PC = Cast<APlayerController>(GetControllerFromActorInfo());
	if (PC == nullptr || PC->IsLocalController() == false)
	{
		return nullptr;
	}

	UCommonPrimaryGameLayout* Layout = UCommonPrimaryGameLayout::GetPrimaryGameLayout(PC->GetLocalPlayer());
	if (Layout == nullptr)
	{
		return nullptr;
	}

	// HUD 위젯이 이 ViewModel에 바인딩되어 있어야 레이아웃이 생성해 둔다. (없으면 nullptr)
	CachedPromptViewModel = Layout->GetViewModel<UInteractionPromptViewModel>(UInteractionPromptViewModel::ViewModelName);
	return CachedPromptViewModel.Get();
}

void UA1Ability_Interact_Scan::WaitForInputStart()
{
	if (UA1AbilityTask_WaitInputStart* InputStartTask = UA1AbilityTask_WaitInputStart::WaitInputStart(this))
	{
		InputStartTask->OnStart.AddDynamic(this, &ThisClass::OnInputStart);
		InputStartTask->ReadyForActivation();
	}
}

void UA1Ability_Interact_Scan::OnInputStart()
{
	UE_LOG(A1AbilityInteractScanLog, Log, TEXT("OnInputStart: CurrentTarget=%s"), *GetNameSafe(CurrentTarget.Get()));

	// 현재 대상이 있으면 실제 실행 어빌리티(UA1Ability_Interact)를 GameplayEvent로 트리거한다.
	// SendGameplayEvent는 로컬(클라)에서 발생하지만, UA1Ability_Interact는 LocalPredicted라
	// 클라 예측과 함께 서버로 활성화 RPC(+TriggerEventData)를 보내 서버 권한에서 결과를 처리한다.
	if (AActor* Target = CurrentTarget.Get())
	{
		FGameplayEventData Payload;
		Payload.EventTag = A1GameplayTags::GameplayEvent_Interact;
		Payload.Instigator = GetAvatarActorFromActorInfo();
		Payload.Target = Target;
		SendGameplayEvent(A1GameplayTags::GameplayEvent_Interact, Payload);
	}

	// 1회용 태스크이므로 다음 입력을 위해 다시 대기한다.
	WaitForInputStart();
}
