#include "A1Ability_MeleeWeaponAttack.h"
#include "Weapon/MeleeWeaponInstance.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "A1GameplayTags.h"
#include "DeveloperPrint.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/A1VitalSet.h"
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

	// 무기의 스태미나 소비량보다 현재 스태미나가 부족하면 공격을 시작할 수 없다.
	const float StaminaCost = WeaponInstance->GetStaminaCost();
	if (StaminaCost > 0.f)
	{
		UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
		if (ASC == nullptr)
		{
			return false;
		}

		bool bFoundAttribute = false;
		const float CurrentStamina = UAbilitySystemBlueprintLibrary::GetFloatAttributeFromAbilitySystemComponent(ASC, UA1VitalSet::GetStaminaAttribute(), bFoundAttribute);
		if (bFoundAttribute == false || CurrentStamina < StaminaCost)
		{
			return false;
		}
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

	// 공격 시작 시 무기별 스태미나를 소비한다. (스태미나 재생 차단은 베이스 UA1Ability_MeleeWeapon에서 처리)
	ApplyStaminaCost();

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

// 현재 장착 무기의 스태미나 소비량을 SetByCaller로 실어 스태미나 소비 GE를 자신에게 적용한다.
// (데미지와 동일한 SetByCaller 방식 — 무기별로 값이 다르고 밸런싱으로 바뀔 수 있어 GE에 고정하지 않는다)
void UA1Ability_MeleeWeaponAttack::ApplyStaminaCost() const
{
	if (StaminaCostEffectClass == nullptr)
	{
		// 소비 GE가 지정되지 않았으면 소비 없이 진행한다. (BP에서 미설정 시 공격만 되고 소비는 되지 않음)
		UE_LOG(A1Ability_MeleeWeaponAttack, Warning, TEXT("ApplyStaminaCost: StaminaCostEffectClass가 설정되지 않아 소비를 건너뜁니다. (%s)"), *GetName());
		return;
	}

	const UMeleeWeaponInstance* WeaponInstance = GetMeleeWeaponInstance();
	if (WeaponInstance == nullptr)
	{
		UE_LOG(A1Ability_MeleeWeaponAttack, Warning, TEXT("ApplyStaminaCost: WeaponInstance를 찾을 수 없어 소비를 건너뜁니다."));
		return;
	}

	const float StaminaCost = WeaponInstance->GetStaminaCost();
	if (StaminaCost <= 0.f)
	{
		// 소비량이 0 이하인 무기는 스태미나를 소비하지 않는다.
		UE_LOG(A1Ability_MeleeWeaponAttack, Warning, TEXT("ApplyStaminaCost: 무기 [%s]의 StaminaCost가 0 이하(%.2f)라 소비를 건너뜁니다."), *GetNameSafe(WeaponInstance), StaminaCost);
		return;
	}

	UCommonAbilitySystemComponent* SourceASC = GetCommonAbilitySystemComponentFromActorInfo();
	if (SourceASC == nullptr)
	{
		UE_LOG(A1Ability_MeleeWeaponAttack, Warning, TEXT("ApplyStaminaCost: ASC를 찾을 수 없어 소비를 건너뜁니다."));
		return;
	}

	// Instant GE라 소유 클라에서도 예측 적용이 가능하도록 예측 윈도우 안에서 자신에게 적용한다.
	FScopedPredictionWindow ScopedPrediction(SourceASC, GetCurrentActivationInfo().GetActivationPredictionKey());

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(StaminaCostEffectClass);
	if (SpecHandle.IsValid() == false)
	{
		UE_LOG(A1Ability_MeleeWeaponAttack, Warning, TEXT("ApplyStaminaCost: GameplayEffectSpec 생성에 실패했습니다. (%s)"), *GetNameSafe(StaminaCostEffectClass));
		return;
	}

	// 무기별 소비량을 SetByCaller.StaminaCost 태그를 키로 스펙에 심는다.
	// GE_Cost_Stamina의 Modifier Op가 Add이므로, 소비(감소)시키려면 음수 값을 실어야 한다.
	// (UE5.8 GE 에디터의 Set by Caller Magnitude에는 별도 계수(Coefficient) 필드가 없어 여기서 부호를 반전한다)
	SpecHandle.Data->SetSetByCallerMagnitude(A1GameplayTags::SetByCaller_StaminaCost, -StaminaCost);

	const FActiveGameplayEffectHandle AppliedHandle = ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle);
	UE_LOG(A1Ability_MeleeWeaponAttack, Log, TEXT("ApplyStaminaCost: 무기 [%s] StaminaCost=%.2f 적용 결과 Handle.IsValid=%d (HasAuthority=%d)"),
		*GetNameSafe(WeaponInstance), StaminaCost, AppliedHandle.IsValid(), HasAuthority(&CurrentActivationInfo));
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