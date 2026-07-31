#include "A1Ability_Interact.h"

#include "A1GameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/Tasks/A1AbilityTask_WaitForTick.h"
#include "Interaction/A1Interactable.h"
#include "Player/A1PlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_Interact)

DEFINE_LOG_CATEGORY(A1AbilityInteractLog);

UA1Ability_Interact::UA1Ability_Interact(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 소유 클라 → 서버 RPC(CommitInteractServer)를 쓰려면 어빌리티 인스턴스가 복제되어야 한다.
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	ActivationPolicy = ECommonAbilityActivationPolicy::OnInputTriggered;

	SetAssetTags(FGameplayTagContainer(A1GameplayTags::Ability_Interact));
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_Interacting);
}

void UA1Ability_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 대상 스냅샷과 근접 감시는 커서를 가진 소유 클라(또는 호스트)만 수행한다.
	// 서버 원격 사본은 여기서 대기하다가 CommitInteractServer RPC를 받아 실행한다.
	if (IsLocallyControlled() == false)
		return;

	AA1PlayerController* PC = Cast<AA1PlayerController>(GetControllerFromActorInfo());
	AActor* Hovered = PC ? PC->GetHoveredInteractable() : nullptr;
	IA1Interactable* Interactable = Cast<IA1Interactable>(Hovered);
	if (Interactable == nullptr)
	{
		// 커서 아래 대상이 없으면 아무것도 하지 않고 종료한다.
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	FA1InteractionQuery Query;
	Query.RequestingAvatar = GetAvatarActorFromActorInfo();
	Query.RequestingController = GetControllerFromActorInfo();

	TArray<FA1InteractionOption> Options;
	Interactable->GatherInteractionOptions(Query, Options);
	if (Interactable->CanInteract(Query) == false || Options.Num() == 0)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	TargetActor = Hovered;
	InteractionRange = Options[0].InteractionRange;

	// 대상 근접까지 매 프레임 거리 감시 (자동 이동 없음, WASD로 접근).
	if (UA1AbilityTask_WaitForTick* TickTask = UA1AbilityTask_WaitForTick::WaitForTick(this))
	{
		TickTask->OnTick.AddDynamic(this, &ThisClass::OnTick);
		TickTask->ReadyForActivation();
	}
}

void UA1Ability_Interact::OnTick(float DeltaTime)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	AActor* Target = TargetActor.Get();
	IA1Interactable* Interactable = Cast<IA1Interactable>(Target);

	// 대상이 사라졌거나 더 이상 상호작용 불가하면 종료.
	if (Avatar == nullptr || Interactable == nullptr)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	FA1InteractionQuery Query;
	Query.RequestingAvatar = Avatar;
	Query.RequestingController = GetControllerFromActorInfo();
	if (Interactable->CanInteract(Query) == false)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	const float DistSq = FVector::DistSquared2D(Avatar->GetActorLocation(), Target->GetActorLocation());
	if (DistSq <= InteractionRange * InteractionRange)
	{
		CommitInteractLocal();
	}
}

void UA1Ability_Interact::CommitInteractLocal()
{
	AActor* Target = TargetActor.Get();
	if (Target == nullptr)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (HasAuthority(&CurrentActivationInfo))
	{
		// 호스트/스탠드얼론: 로컬이 곧 서버.
		PerformInteractionAuth(Target);
	}
	else
	{
		// 소유 클라: 서버에 실행 요청.
		CommitInteractServer(Target);
	}

	// 로컬 인스턴스는 여기서 종료. (서버 인스턴스는 RPC 처리 후 자체 종료)
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UA1Ability_Interact::CommitInteractServer_Implementation(AActor* Target)
{
	PerformInteractionAuth(Target);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UA1Ability_Interact::PerformInteractionAuth(AActor* Target)
{
	if (HasAuthority(&CurrentActivationInfo) == false)
		return;

	AActor* Avatar = GetAvatarActorFromActorInfo();
	IA1Interactable* Interactable = Cast<IA1Interactable>(Target);
	if (Avatar == nullptr || Interactable == nullptr)
		return;

	FA1InteractionQuery Query;
	Query.RequestingAvatar = Avatar;
	Query.RequestingController = GetControllerFromActorInfo();
	if (Interactable->CanInteract(Query) == false)
		return;

	// 서버 거리 재검증(관대한 허용 오차). 클라 예측/지연으로 살짝 벗어난 경우를 허용한다.
	const float ServerRange = GetInteractionRangeFor(Target) + ServerRangeTolerance;
	const float DistSq = FVector::DistSquared2D(Avatar->GetActorLocation(), Target->GetActorLocation());
	if (DistSq > ServerRange * ServerRange)
	{
		UE_LOG(A1AbilityInteractLog, Verbose, TEXT("서버 거리 재검증 실패: %s"), *GetNameSafe(Target));
		return;
	}

	// 실제 결과 처리 (문 열기/아이템 획득/데미지 등)
	Interactable->OnInteractAuth(Avatar);

	// BP/GA 확장을 위해 대상에게 GameplayEvent도 통지한다. (예시 액터는 OnInteractAuth만 사용)
	TArray<FA1InteractionOption> Options;
	Interactable->GatherInteractionOptions(Query, Options);
	if (Options.Num() > 0 && Options[0].InteractEventTag.IsValid())
	{
		FGameplayEventData Payload;
		Payload.EventTag = Options[0].InteractEventTag;
		Payload.Instigator = Avatar;
		Payload.Target = Target;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Target, Options[0].InteractEventTag, Payload);
	}

	UE_LOG(A1AbilityInteractLog, Log, TEXT("상호작용 실행: %s ← %s"), *GetNameSafe(Target), *GetNameSafe(Avatar));
}

float UA1Ability_Interact::GetInteractionRangeFor(AActor* Target) const
{
	IA1Interactable* Interactable = Cast<IA1Interactable>(Target);
	if (Interactable == nullptr)
		return InteractionRange;

	FA1InteractionQuery Query;
	Query.RequestingAvatar = GetAvatarActorFromActorInfo();
	Query.RequestingController = GetControllerFromActorInfo();

	TArray<FA1InteractionOption> Options;
	Interactable->GatherInteractionOptions(Query, Options);
	return Options.Num() > 0 ? Options[0].InteractionRange : InteractionRange;
}
