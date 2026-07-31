#include "A1Ability_Sprint_Active.h"

#include "A1GameplayTags.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "AbilitySystem/Tasks/A1AbilityTask_WaitForTick.h"
#include "Engine/World.h"
#include "Game/CommonCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_Sprint_Active)

UA1Ability_Sprint_Active::UA1Ability_Sprint_Active(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Check가 보낸 GameplayEvent로만 발동한다. 입력으로 직접 발동하지 않는다.
	ActivationPolicy = ECommonAbilityActivationPolicy::Manual;

	// EndAbility는 기본적으로 양방향 복제된다 (서버→클라: ClientEndAbility RPC / 클라→서버: ServerEndAbility RPC).
	// 아래 두 설정으로 "서버만" 종료 권위를 갖도록 한쪽 방향을 막는다.
	//  - bServerRespectsRemoteAbilityCancellation = false : 서버가 클라이언트발 취소/종료 요청을 무시한다.
	//  - NetSecurityPolicy = ServerOnlyTermination         : 클라이언트의 ServerEndAbility RPC 자체를 거부한다.
	// 그 결과 클라에서 EndAbility를 불러도 "클라 로컬 인스턴스"만 종료될 뿐 서버 인스턴스는 끝나지 않는다.
	// 서버는 자신의 OnTick/OnSprintCommitTick 판정으로 독립적으로 종료하고, 그 종료가 다시 클라로 복제되어 최종 동기화된다.
	// (클라를 신뢰해 시작은 예측하되, 지속 여부의 최종 결정권은 항상 서버가 갖도록 하기 위함)
	bServerRespectsRemoteAbilityCancellation = false;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnlyTermination;

	SetAssetTags(FGameplayTagContainer(A1GameplayTags::Ability_Sprint_Active));
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_Sprint);
	// 스프린트 중에는 스태미나 재생을 즉시 멈춘다. (GE_Stamina_Regen이 이 태그를 Ignore로 감시)
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_StaminaRegen_Blocked);

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = A1GameplayTags::Ability_Sprint_Active;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UA1Ability_Sprint_Active::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (TriggerEventData == nullptr)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	if (K2_CheckAbilityCost() == false || K2_CheckAbilityCooldown() == false)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	if (UA1AbilityTask_WaitForTick* TickTask = UA1AbilityTask_WaitForTick::WaitForTick(this))
	{
		TickTask->OnTick.AddDynamic(this, &ThisClass::OnTick);
		TickTask->ReadyForActivation();
	}

	if (UAbilityTask_WaitInputRelease* InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true))
	{
		InputReleaseTask->OnRelease.AddDynamic(this, &ThisClass::OnInputReleased);
		InputReleaseTask->ReadyForActivation();
	}

	// 이동속도 상승 (로컬 배율). 서버·소유 클라 각각 자기 쪽에서 적용된다.
	SetSprintSpeedLocal(true);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(SprintCommitTimerHandle, this, &ThisClass::OnSprintCommitTick, CommitInterval, true);
	}
}

void UA1Ability_Sprint_Active::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SprintCommitTimerHandle);
	}

	// 이동속도 원복
	SetSprintSpeedLocal(false);

	// 회복 억제 효과 부여: Status.StaminaRegen.Blocked를 RegenBlockDuration초 동안 부여해
	// 위에서 곧 Super::EndAbility가 제거할 ActivationOwnedTags(Status.StaminaRegen.Blocked)의 공백을 이어받는다.
	// GE 에셋 자체의 Duration 값과 무관하게 SetDuration으로 강제하므로, BP에는 Duration Policy만 "Has Duration"으로
	// 맞춰두면 된다 (에셋에 적어둔 구체적 시간 값은 무시됨).
	if (RecoveryBlockEffectClass)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(RecoveryBlockEffectClass, 1.f);
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->SetDuration(RegenBlockDuration, true);
			(void)ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UA1Ability_Sprint_Active::OnTick(float DeltaTime)
{
	ACommonCharacter* Character = GetCommonCharacterFromActorInfo();
	UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	if (Movement == nullptr)
		return;

	// 정지 판정: 서버는 실제 속도, 원격 소유 클라는 입력 벡터 기준
	//
	// 클라(else 분기)에서 EndAbility를 불러도, 위 생성자의 ServerOnlyTermination 정책 때문에
	// 서버 인스턴스는 끝나지 않는다 — 클라 쪽은 태스크 정리·SetSprintSpeedLocal(false) 등 "로컬 마무리"만 즉시 실행되는
	// 예측성 종료이고, 실제 최종 종료는 아래 authority 분기(서버의 실제 Velocity 판정)가 결정해 클라로 복제해준다.
	// 즉 클라의 EndAbility 호출이 "무시되는" 것이 아니라, 서버로의 전파만 막혀 있는 것이다.
	bool bStopped = false;
	if (HasAuthority(&CurrentActivationInfo))
	{
		const FVector PlanarVelocity = Movement->Velocity * FVector(1.f, 1.f, 0.f);
		bStopped = PlanarVelocity.Size() <= StopSpeedThreshold;
	}
	else
	{
		bStopped = Character->GetLastMovementInputVector().IsNearlyZero();
	}

	if (bStopped)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UA1Ability_Sprint_Active::OnInputReleased(float TimeHeld)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UA1Ability_Sprint_Active::OnSprintCommitTick()
{
	if (K2_CommitAbilityCost() == false)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(SprintCommitTimerHandle);
		}
		K2_CommitAbilityCooldown(false, true);
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UA1Ability_Sprint_Active::SetSprintSpeedLocal(bool bEnable)
{
	ACommonCharacter* Character = GetCommonCharacterFromActorInfo();
	UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	if (Movement == nullptr)
		return;

	if (bEnable)
	{
		// 이미 적용돼 있으면 CachedMaxWalkSpeed가 배율 적용된 값으로 덮이지 않도록 방지한다.
		if (bSprintSpeedApplied)
			return;

		CachedMaxWalkSpeed = Movement->MaxWalkSpeed;
		Movement->MaxWalkSpeed = CachedMaxWalkSpeed * SprintSpeedMultiplier;
		bSprintSpeedApplied = true;
	}
	else
	{
		if (bSprintSpeedApplied == false)
			return;

		Movement->MaxWalkSpeed = CachedMaxWalkSpeed;
		bSprintSpeedApplied = false;
	}
}
