#include "A1Ability_Skill_GroundBreaker.h"
#include "A1GameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimMontage.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Game/CommonCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/RootMotionSource.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_Skill_GroundBreaker)

DEFINE_LOG_CATEGORY(A1Ability_Skill_GroundBreakerLog);

// TODO: Cooldown 및 Mana Check 및 몽타주 Notify

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

	// 시전 중에는 WASD 이동 "입력"만 막는다. (EndAbility에서 원복)
	// 이동 모드는 MOVE_Walking으로 유지되므로 몽타주 루트 모션이 캐릭터를 정면으로 전진시킬 수 있다.
	// DisableMovement(MOVE_None)를 쓰면 CharacterMovementComponent가 이동 계산을 통째로 건너뛰어 루트 모션도 무시된다.
	SetMoveInputBlockedLocal(true);

	if (UAbilityTask_PlayMontageAndWait* GroundBreakerMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("GroundBreakerMontage"), GroundBreakerMontage, 1.f, NAME_None, true))
	{
		GroundBreakerMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
		GroundBreakerMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageFinished);
		GroundBreakerMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageFinished);
		GroundBreakerMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageFinished);
		GroundBreakerMontageTask->ReadyForActivation();
	}

	// 몽타주에 루트 모션이 없는 경우를 위한 대체 전진 (ForwardMoveDistance가 0이면 아무 것도 하지 않는다)
	StartForwardRootMotion();

	if (UAbilityTask_WaitGameplayEvent* GroundBreakerBeginTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, A1GameplayTags::GameplayEvent_Montage_Begin, nullptr, true, true))
	{
		GroundBreakerBeginTask->EventReceived.AddDynamic(this, &ThisClass::OnGroundBreakerBegin);
		GroundBreakerBeginTask->ReadyForActivation();
	}

	if (GroundBreakerMontage && GroundBreakerMontage->HasRootMotion() == false && ForwardMoveDistance <= 0.f)
	{
		UE_LOG(A1Ability_Skill_GroundBreakerLog, Warning, TEXT("[%s] 몽타주에 루트 모션이 없고 ForwardMoveDistance도 0이라 전진하지 않는다. 몽타주의 Enable Root Motion을 켜거나 ForwardMoveDistance를 설정할 것."), *GetNameSafe(GroundBreakerMontage));
	}
}

void UA1Ability_Skill_GroundBreaker::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 정상 종료·취소·중단 모두 이곳을 거치므로 막아두었던 이동 입력을 원복한다.
	SetMoveInputBlockedLocal(false);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

/**
 * 몽타주 자체에 루트 모션이 없을 때 쓰는 보조 전진.
 * 탑다운(WASD) 조작이라 시전 시점에 캐릭터가 바라보는 방향이 곧 스킬 방향이므로 그 방향으로 밀어준다.
 */
void UA1Ability_Skill_GroundBreaker::StartForwardRootMotion()
{
	if (ForwardMoveDistance <= 0.f || ForwardMoveDuration <= 0.f)
		return;

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (AvatarActor == nullptr)
		return;

	// 탑다운이므로 수평 성분만 사용한다. (경사면에서 위/아래로 튀지 않도록)
	FVector ForwardDirection = AvatarActor->GetActorForwardVector();
	ForwardDirection.Z = 0.f;
	if (ForwardDirection.Normalize() == false)
		return;

	// ConstantForce의 Strength는 cm/s로 적용되므로 "거리 / 시간"이 곧 속도가 된다.
	const float Strength = ForwardMoveDistance / ForwardMoveDuration;

	if (UAbilityTask_ApplyRootMotionConstantForce* ForwardMoveTask = UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
			this, TEXT("GroundBreakerForwardMove"), ForwardDirection, Strength, ForwardMoveDuration,
			false, nullptr, ERootMotionFinishVelocityMode::ClampVelocity, FVector::ZeroVector, 0.f, true))
	{
		ForwardMoveTask->ReadyForActivation();
	}
}

/**
 * 충돌 판정 박스를 화면에 그린다. bDrawDebugShape가 켜져 있을 때만 동작하며 서버·클라이언트 모두에서 호출된다.
 * 히트 여부를 알 수 있는 서버에서만 HitColor가 쓰이고, 클라이언트는 항상 TraceColor로 그린다.
 */
void UA1Ability_Skill_GroundBreaker::DrawDebugShape(const FVector& Center, const FVector& HalfSize, const FRotator& Orientation, bool bHasHit) const
{
#if ENABLE_DRAW_DEBUG
	if (bDrawDebugShape == false)
		return;

	const UWorld* World = GetWorld();
	if (World == nullptr)
		return;

	const FColor Color = bHasHit ? HitColor : TraceColor;
	DrawDebugBox(World, Center, HalfSize, Orientation.Quaternion(), Color, false, DebugDrawDuration);
#endif
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

	// 데미지 판정은 서버 전용이지만 디버그 박스는 클라이언트에서도 볼 수 있어야 하므로,
	// 권한 체크 전에 판정 영역을 먼저 그린다. (클라는 히트 결과를 모르므로 TraceColor로 그린다)
	if (HasAuthority(&CurrentActivationInfo) == false)
	{
		DrawDebugShape(Start, HalfSize, Orientation, false);
		return;
	}

	TArray<FHitResult> OutHitResults;
	UKismetSystemLibrary::BoxTraceMultiForObjects(GetWorld(), Start, End, HalfSize, Orientation, ObjectTypes, false, ActorsToIgnore, EDrawDebugTrace::None, OutHitResults, true);

	DrawDebugShape(Start, HalfSize, Orientation, OutHitResults.Num() > 0);

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
