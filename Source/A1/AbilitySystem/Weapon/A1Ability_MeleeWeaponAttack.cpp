#include "A1Ability_MeleeWeaponAttack.h"
#include "Weapon/MeleeWeaponInstance.h"
#include "AbilitySystemComponent.h"
#include "A1GameplayTags.h"
#include "DeveloperPrint.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/CommonAbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/A1Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_MeleeWeaponAttack)

DEFINE_LOG_CATEGORY(A1Ability_MeleeWeaponAttack);

UA1Ability_MeleeWeaponAttack::UA1Ability_MeleeWeaponAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)

{
	// TODO: Network
}

bool UA1Ability_MeleeWeaponAttack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const UMeleeWeaponInstance* WeaponInstance = GetMeleeWeaponInstance();
	UAnimMontage* AttackMontage = WeaponInstance->GetAttackMontage(ComboIndex);
	if (AttackMontage == nullptr)
	{
		return false;
	}

	return true;
}

void UA1Ability_MeleeWeaponAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// BP에서 활성화한 경우에만 공격 중 이동 방향으로의 자동 회전을 끈다. (서버·소유 클라 각각에서 로컬 적용)
	if (bDisableOrientRotationDuringAttack)
	{
		// EndAbility에서 원복할 수 있도록 실행되는 쪽의 현재 값을 캐시한다.
		if (const ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
		{
			if (const UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
			{
				bCachedOrientRotationToMovement = MovementComp->bOrientRotationToMovement;
			}
		}
		SetOrientRotationToMovementLocal(false);
	}

	UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
	check(ASC);

	const UMeleeWeaponInstance* WeaponInstance = GetMeleeWeaponInstance();
	UAnimMontage* AttackMontage = WeaponInstance->GetAttackMontage(ComboIndex);

	if (UAbilityTask_WaitGameplayEvent* GameplayEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, A1GameplayTags::GameplayEvent_Trace, nullptr, false, true))
	{
		GameplayEventTask->EventReceived.AddDynamic(this, &ThisClass::OnTargetDataReady);
		GameplayEventTask->ReadyForActivation();
	}

	if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("MeleeAttack"), AttackMontage, 1.0f, NAME_None, false, 1.f, 0.f, false))
	{
		PlayMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
		PlayMontageTask->ReadyForActivation();
	}

	if (UAbilityTask_WaitGameplayEvent* GameplayEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, A1GameplayTags::GameplayEvent_Montage_End, nullptr, true, true))
	{
		GameplayEventTask->EventReceived.AddDynamic(this, &ThisClass::OnMontageEventTriggered);
		GameplayEventTask->ReadyForActivation();
	}
}

void UA1Ability_MeleeWeaponAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 정상 종료·취소 모두 이곳을 거치므로 캐시해 둔 원래 회전 설정으로 되돌린다.
	if (bDisableOrientRotationDuringAttack)
	{
		SetOrientRotationToMovementLocal(bCachedOrientRotationToMovement);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UA1Ability_MeleeWeaponAttack::HandleMontageEvent(FGameplayEventData Payload)
{
	OnMontageFinished();
}

// PerformTrace(AnimNotifyState)가 히트 시 보내는 GameplayEvent_Trace를 받아 실행되는 콜백.
// 트레이스가 서버 전용이라 이 함수도 서버에서만 호출된다. Payload의 TargetData에서 대상을 추려 데미지 GE를 적용한다.
void UA1Ability_MeleeWeaponAttack::OnTargetDataReady(FGameplayEventData Payload)
{
	UCommonAbilitySystemComponent* SourceASC = GetCommonAbilitySystemComponentFromActorInfo();
	if (SourceASC == nullptr)
		return;
	
	// 어빌리티 스펙이 아직 유효할 때만 처리한다. (활성화가 이미 끝났으면 무시)
	if (SourceASC->FindAbilitySpecFromHandle(CurrentSpecHandle))
	{
		// PerformTrace가 보낸 이벤트의 Payload에서 TargetData(맞은 대상들의 HitResult 묶음)를 로컬로 가져온다.
		// MoveTemp으로 소유권을 옮겨 복사를 피한다. (이벤트/타깃데이터가 전부 서버 로컬이라 복제는 필요 없음)
		FGameplayAbilityTargetDataHandle LocalTargetDataHandle(MoveTemp(const_cast<FGameplayAbilityTargetDataHandle&>(Payload.TargetData)));
	
		// TargetData를 훑어 "캐릭터인 대상 & 이번 활성화에서 아직 안 맞은 대상"의 인덱스만 골라낸다.
		TArray<int32> CharacterHitIndexes;
		ParseTargetData(LocalTargetDataHandle, CharacterHitIndexes);

		// 현재 콤보 인덱스에 해당하는 무기 데미지 값. (아래 ProcessHitResult에서 SetByCaller로 GE에 전달)
		UMeleeWeaponInstance* WeaponInstance = GetMeleeWeaponInstance();
		float Damage = WeaponInstance->GetComboDamage(ComboIndex);
		
		// 골라낸 인덱스별로 실제 HitResult를 꺼내 데미지 처리를 수행한다.
		// (인덱스는 ParseTargetData가 GetHitResult() != null 인 항목만 담았으므로 역참조가 안전하다)
		for (int32 CharacterHitIndex : CharacterHitIndexes)
		{
			FHitResult HitResult = *LocalTargetDataHandle.Data[CharacterHitIndex]->GetHitResult();
			ProcessHitResult(HitResult, Damage);
		}
	}
}

void UA1Ability_MeleeWeaponAttack::OnMontageEventTriggered(FGameplayEventData Payload)
{
	HandleMontageEvent(Payload);
}

void UA1Ability_MeleeWeaponAttack::OnMontageFinished()
{
	if (HasAuthority(&CurrentActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UA1Ability_MeleeWeaponAttack::SetOrientRotationToMovementLocal(bool bNewOrient) const
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		return;
	}

	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (!MovementComp)
	{
		return;
	}

	MovementComp->bOrientRotationToMovement = bNewOrient;
}