// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Weapon/A1Ability_MeleeWeapon.h"

#include "A1GameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/CommonAbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Player/A1Character.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_MeleeWeapon)

DEFINE_LOG_CATEGORY(A1Ability_MeleeWeapon)

UA1Ability_MeleeWeapon::UA1Ability_MeleeWeapon(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_StaminaRegen_Blocked);
}

void UA1Ability_MeleeWeapon::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ResetHitActors();
}

/**
 * TargetData의 각 HitResult를 순회하며 "AA1Character인 대상"이면서 "이번 활성화에서 아직 안 맞은 대상"만
 * OutCharacterHitIndexes에 인덱스로 담는다. (CachedHitActors로 중복 히트 방지 + 캐릭터 필터링)
 */
void UA1Ability_MeleeWeapon::ParseTargetData(const FGameplayAbilityTargetDataHandle& InTargetDataHandle, TArray<int32>& OutCharacterHitIndexes)
{
	for (int32 i = 0; i < InTargetDataHandle.Data.Num(); i++)
	{
		const TSharedPtr<FGameplayAbilityTargetData>& TargetData = InTargetDataHandle.Data[i];

		// HitResult가 들어 있는 TargetData만 처리한다. (GetHitResult()가 null이면 스킵)
		if (FHitResult* HitResult = const_cast<FHitResult*>(TargetData->GetHitResult()))
		{
			if (AActor* HitActor = HitResult->GetActor())
			{
				// 맞은 것이 캐릭터가 아니면(예: 무기/부위 콜리전) 그 소유자에서 캐릭터를 찾는다.
				AA1Character* TargetCharacter = Cast<AA1Character>(HitActor);
				if (TargetCharacter == nullptr)
				{
					TargetCharacter = Cast<AA1Character>(HitActor->GetOwner());
				}

				// 이번 어빌리티 활성화에서 이미 맞은 대상이면 건너뛴다. (한 번의 공격에 한 대상 한 번만 히트)
				AActor* SelectedActor = TargetCharacter ? TargetCharacter : HitActor;
				if (CachedHitActors.Contains(SelectedActor))
					continue;

				CachedHitActors.Add(SelectedActor);

				if (TargetCharacter)
				{
					OutCharacterHitIndexes.Add(i);
				}
			}
		}
	}
}

/**
 * 단일 HitResult 대상에게 데미지 GameplayEffect를 적용한다. (서버 권위에서만 실제 적용)
 * BaseDamage는 SetByCaller로 스펙에 실어 보내고, 실제 데미지 계산은 GE의 Execution(UA1DamageExecution)이 수행한다.
 */
void UA1Ability_MeleeWeapon::ProcessHitResult(const FHitResult& HitResult, float Damage) const
{
	UCommonAbilitySystemComponent* SourceASC = GetCommonAbilitySystemComponentFromActorInfo();
	if (SourceASC == nullptr)
		return;
	
	FScopedPredictionWindow	ScopedPrediction(SourceASC, GetCurrentActivationInfo().GetActivationPredictionKey());
	
	// TODO: GameplayCue
	//FGameplayCueParameters SourceCueParams;
	//SourceCueParams.Location = HitResult.ImpactPoint;
	//SourceCueParams.Normal = HitResult.ImpactNormal;
	//SourceCueParams.PhysicalMaterial = HitResult.PhysMaterial;
	//SourceASC->ExecuteGameplayCue(D1GameplayTags::GameplayCue_Weapon_Impact, SourceCueParams);
	
	if (HasAuthority(&CurrentActivationInfo))
	{
		// GE를 적용할 "대상"을 지정하는 TargetData. 맞은 액터 하나로 만든다.
		FGameplayAbilityTargetDataHandle TargetDataHandle = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitResult.GetActor());

		// GE가 지정되지 않았으면 스펙의 Data가 무효라 이후 역참조에서 크래시하므로 여기서 막는다.
		if (DamageEffectClass == nullptr)
		{
			UE_LOG(A1Ability_MeleeWeapon, Warning, TEXT("DamageEffectClass가 설정되지 않았습니다."));
			return;
		}
		
		// DamageEffectClass(BP에서 지정한 GE)로부터 런타임 스펙을 만든다.
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass);

		// 이펙트 컨텍스트: Instigator(Owner)/EffectCauser(Avatar)는 MakeEffectContext가 이미 세팅한다.
		// 여기에 HitResult를 실어두면 Execution/GameplayCue에서 타격 지점·대상 등을 참조할 수 있다.
		FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
		EffectContextHandle.AddHitResult(HitResult);
		//EffectContextHandle.AddInstigator(SourceASC->AbilityActorInfo->OwnerActor.Get()); // TODO: 원하는 무기에 맞춰서 처리
		EffectSpecHandle.Data->SetContext(EffectContextHandle);

		// BaseDamage 값을 SetByCaller_BaseDamage 태그를 "키"로 스펙에 심는다.
		// UA1DamageExecution이 같은 태그로 GetSetByCallerMagnitude 해서 이 값을 꺼내 데미지를 계산한다.
		EffectSpecHandle.Data->SetSetByCallerMagnitude(A1GameplayTags::SetByCaller_BaseDamage, Damage);

		// 완성된 스펙을 대상에게 적용한다. 이 시점에 GE의 Execution(UA1DamageExecution)이 실행되어
		// Damage 메타 어트리뷰트가 변하고, UA1VitalSet::PostGameplayEffectExecute에서 Health로 반영된다.
		// 반환되는 ActiveGEHandle 배열은 즉발 데미지라 쓸 일이 없어 (void)로 명시적으로 버린다.
		(void)ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, TargetDataHandle);
	}
}

void UA1Ability_MeleeWeapon::ResetHitActors()
{
	CachedHitActors.Reset();
};

/**
 * DisableMovement()로 MOVE_None을 만들면 CharacterMovementComponent가 이동 계산 자체를 건너뛰어
 * 몽타주 루트 모션·RootMotionSource도 전부 무시된다. (공중에서는 중력까지 멈춘다)
 * 그래서 이동 모드는 그대로 두고 AController의 이동 입력만 막아(AddMovementInput이 무시된다)
 * "조작은 불가 + 애니메이션 전진은 유지" 상태를 만든다.
 */
void UA1Ability_MeleeWeapon::SetMoveInputBlockedLocal(bool bBlocked)
{
	// SetIgnoreMoveInput은 누적 카운터라 차단/해제 호출 짝이 반드시 맞아야 한다.
	// 시전 중 아바타가 죽어 교체되어도 원래 컨트롤러를 되돌릴 수 있도록 차단한 컨트롤러를 기억해 둔다.
	if (bBlocked == false)
	{
		if (AController* BlockedController = MoveInputBlockedController.Get())
		{
			BlockedController->SetIgnoreMoveInput(false);
		}

		MoveInputBlockedController.Reset();
		return;
	}

	// 이미 차단 중이면 카운터를 중복으로 올리지 않는다.
	if (MoveInputBlockedController.IsValid())
		return;

	APawn* Pawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (Pawn == nullptr)
		return;

	AController* Controller = Pawn->GetController();
	if (Controller == nullptr)
		return;

	Controller->SetIgnoreMoveInput(true);
	MoveInputBlockedController = Controller;

	if (UCharacterMovementComponent* MovementComp = Cast<UCharacterMovementComponent>(Pawn->GetMovementComponent()))
	{
		// 차단 직전까지 쌓인 속도·입력을 지워 시전 중에는 루트 모션만 캐릭터를 움직이게 한다.
		MovementComp->StopMovementImmediately();
		Pawn->ConsumeMovementInputVector();
	}
}
