// Copyright Epic Games, Inc. All Rights Reserved.

#include "A1Ability_Death.h"

#include "A1GameplayTags.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"
#include "Game/CommonCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameplayEffectTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_Death)

DEFINE_LOG_CATEGORY(A1AbilityDeathLog);

UA1Ability_Death::UA1Ability_Death(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	// 입력이 아니라 UA1VitalSet이 보내는 GameplayEvent.Death로 트리거된다.
	ActivationPolicy = ECommonAbilityActivationPolicy::Manual;
	ActivationGroup = ECommonAbilityActivationGroup::Exclusive_Blocking;

	SetAssetTags(FGameplayTagContainer(A1GameplayTags::Ability_Death));

	// 이미 사망 처리된 대상이 GameplayEvent.Death를 다시 받아도 중복 활성화되지 않도록 차단한다.
	ActivationBlockedTags.AddTag(A1GameplayTags::Status_Death);

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = A1GameplayTags::GameplayEvent_Death;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UA1Ability_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	check(ActorInfo);

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	check(ASC);

	// Exclusive_Blocking으로 활성화되는 순간 CommonAbilitySystemComponent가 Exclusive_Replaceable
	// Ability(콤보 공격 등)를 자동 취소하므로 여기서 따로 처리하지 않는다. Interact_Scan 같은
	// Independent(패시브) Ability는 절대 취소하면 안 된다 (OnSpawn 재활성화 안전장치와 충돌해 하이라이트가 깜빡였다).

	// 시체 판정용 영구 태그(Ability 종료 후에도 유지되어야 해서 루즈 태그로 직접 설정).
	// TagRepState를 반드시 TagAndCountToAll로 줘야 한다 — 기본값(None)은 이 프로젝트가 쓰는
	// Iris 리플리케이션 하에서 복제가 전혀 안 되어, 시체를 보는 다른 클라에 태그가 전달되지 않는다.
	ASC->SetLooseGameplayTagCount(A1GameplayTags::Status_Death, 1, EGameplayTagReplicationState::TagAndCountToAll);

	// 죽은 뒤에는 어떤 것도 이 Ability를 끊을 수 없어야 한다. (사망 몽타주 재생 중 취소 방지)
	SetCanBeCanceled(false);

	// 이동 불가 처리. Controller/CharacterMovementComponent는 서버·소유 클라(죽은 본인)에만 존재하므로
	// ServerInitiated인 이 Ability(서버+소유 클라에서만 실행)에서 처리하는 것으로 충분하다 — 다른 클라는
	// CharacterMovementComponent 자체의 위치 복제로 "멈춰있는 캐릭터" 결과만 받아본다.
	if (ACommonCharacter* Character = GetCommonCharacterFromActorInfo())
	{
		if (AController* Controller = Character->GetController())
		{
			Controller->SetIgnoreMoveInput(true);
		}

		if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
		{
			// 수평 속도만 지운다. Z는 남겨서 공중에서 죽었다면 낙하를 자연스럽게 이어간다.
			MovementComp->Velocity = FVector(0.f, 0.f, MovementComp->Velocity.Z);
			MovementComp->UpdateComponentVelocity();
		}
	}

	UE_LOG(A1AbilityDeathLog, Log, TEXT("%s 사망 처리"), *GetNameSafe(GetAvatarActorFromActorInfo()));

	// 사망 몽타주 재생 후 종료한다. AbilitySystemComponent::PlayMontage가 세팅하는 RepAnimMontageInfo는
	// COND_None으로 복제되므로, 서버(권위)에서 재생을 시작하면 이 Ability를 실행하지 않는 다른 클라
	// (시체를 보는 쪽)에도 몽타주가 그대로 보인다.
	UAbilityTask_PlayMontageAndWait* Task = DeathMontage ? UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("A1Ability_Death_Montage"), DeathMontage) : nullptr;
	if (Task != nullptr)
	{
		Task->OnCompleted.AddDynamic(this, &ThisClass::OnDeathMontageFinished);
		Task->OnBlendOut.AddDynamic(this, &ThisClass::OnDeathMontageFinished);
		Task->OnInterrupted.AddDynamic(this, &ThisClass::OnDeathMontageFinished);
		Task->OnCancelled.AddDynamic(this, &ThisClass::OnDeathMontageFinished);
		Task->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UA1Ability_Death::OnDeathMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
