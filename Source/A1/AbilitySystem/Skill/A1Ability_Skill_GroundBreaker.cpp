#include "A1Ability_Skill_GroundBreaker.h"
#include "A1GameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Game/CommonCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_Skill_GroundBreaker)

// TODO: 데미지 처리 및 Cooldown 및 Mana Check 및 몽타주 Notify

UA1Ability_Skill_GroundBreaker::UA1Ability_Skill_GroundBreaker(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetAssetTags(FGameplayTagContainer(A1GameplayTags::Ability_Attack_Skill_1));
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_RejectHitReact);
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_Skill);
}

void UA1Ability_Skill_GroundBreaker::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (K2_CheckAbilityCooldown() == false || K2_CheckAbilityCost() == false)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo) == false)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	// 시전 중에는 캐릭터가 움직이지 못하도록 이동을 정지시킨다. (EndAbility에서 원복)
	SetMovementFrozenLocal(true);

	if (UAbilityTask_PlayMontageAndWait* GroundBreakerMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("GroundBreakerMontage"), GroundBreakerMontage, 1.f, NAME_None, true))
	{
		GroundBreakerMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
		GroundBreakerMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageFinished);
		GroundBreakerMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageFinished);
		GroundBreakerMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageFinished);
		GroundBreakerMontageTask->ReadyForActivation();
	}

	if (UAbilityTask_WaitGameplayEvent* GroundBreakerBeginTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, A1GameplayTags::GameplayEvent_Montage_Begin, nullptr, true, true))
	{
		GroundBreakerBeginTask->EventReceived.AddDynamic(this, &ThisClass::OnGroundBreakerBegin);
		GroundBreakerBeginTask->ReadyForActivation();
	}
}

void UA1Ability_Skill_GroundBreaker::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 정상 종료·취소·중단 모두 이곳을 거치므로 정지시켰던 이동을 원복한다.
	SetMovementFrozenLocal(false);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UA1Ability_Skill_GroundBreaker::OnGroundBreakerBegin(FGameplayEventData Payload)
{
	ExecuteGroundBreaker();
}

void UA1Ability_Skill_GroundBreaker::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UA1Ability_Skill_GroundBreaker::ExecuteGroundBreaker()
{
	if (HasAuthority(&CurrentActivationInfo) == false)
		return;

	ACommonCharacter* CommonCharacter = GetCommonCharacterFromActorInfo();
	if (CommonCharacter == nullptr)
		return;

	const float ScaledCapsuleRadius = CommonCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const float ScaledCapsuleHalfHeight = CommonCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	// 캐릭터 정면 DistanceOffset 지점에서 박스 한 번으로 훑는다. (Start == End 인 단일 위치 트레이스)
	const FVector Forward = CommonCharacter->GetActorForwardVector();
	const FVector Start = CommonCharacter->GetActorLocation() + (Forward * DistanceOffset);
	const FVector End = Start;
	const FVector HalfSize = FVector(ScaledCapsuleRadius * 3.f, ScaledCapsuleRadius * 3.f, ScaledCapsuleHalfHeight);
	const FRotator Orientation = UKismetMathLibrary::MakeRotFromX(Forward);
	const TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes = { UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn) };
	const TArray<AActor*> ActorsToIgnore = { CommonCharacter };

	TArray<FHitResult> OutHitResults;
	UKismetSystemLibrary::BoxTraceMultiForObjects(GetWorld(), Start, End, HalfSize, Orientation, ObjectTypes, false, ActorsToIgnore, EDrawDebugTrace::None, OutHitResults, true);
	
	// 한 대상이 여러 컴포넌트로 잡혀 중복될 수 있으므로 캐릭터 단위로 한 번만 처리한다.
	TSet<ACommonCharacter*> ProcessedCharacters;
	for (const FHitResult& HitResult : OutHitResults)
	{
		ACommonCharacter* HitCharacter = Cast<ACommonCharacter>(HitResult.GetActor());
		if (HitCharacter == nullptr)
			continue;
	
		bool bAlreadyProcessed = false;
		ProcessedCharacters.Add(HitCharacter, &bAlreadyProcessed);
		if (bAlreadyProcessed)
			continue;
	
		ProcessHitResult(HitResult, Damage);
	
		FGameplayEventData EventData;
		EventData.EventMagnitude = StunDruation;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(HitCharacter, A1GameplayTags::GameplayEvent_Stun, EventData);
	}
}
