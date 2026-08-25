// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/CommonGameplayAbility.h"
#include "A1Ability_Death.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1AbilityDeathLog, Log, All);

class UAnimMontage;

/**
 * UA1Ability_Death
 *
 * Health가 0 이하가 되면 UA1VitalSet::PostAttributeChange가 소유 ASC로 GameplayEvent.Death를
 * 보내 트리거되는 사망 처리 전담 Ability다 (Manual/GameplayEvent 트리거, ServerInitiated).
 *
 * ActivationGroup = Exclusive_Blocking으로 활성화되므로, 활성화되는 순간 진행 중이던
 * Exclusive_Replaceable Ability(콤보 공격 등)는 CommonAbilitySystemComponent가 자동으로
 * 취소한다(NotifyAbilityActivated → AddAbilityToActivationGroup). Interact_Scan처럼 죽음과
 * 무관하게 계속 떠 있어야 하는 Independent(패시브) Ability는 절대 여기서 직접 취소하면 안 된다
 * — ASC->CancelAbilities로 전부 취소해버리면 OnSpawn+LocalOnly Ability의 재활성화 안전장치
 * (TryActivateLocalOnlyAbilitiesOnSpawn)가 즉시 다시 살려내면서 하이라이트가 깜빡이는 버그가 난다.
 *
 * ASC에 Status.Death 루즈 태그를 영구히 남긴다. ActivationOwnedTags와 달리 이 태그는 Ability가
 * 끝난 뒤에도 남아있어야 하므로 직접 SetLooseGameplayTagCount로 설정한다.
 * AA1Character::CanInteract는 이 태그 하나로 "시체 상태" 여부를 판정한다.
 *
 * 이 태그는 반드시 EGameplayTagReplicationState::TagAndCountToAll로 설정해야 한다. 이 프로젝트는
 * Iris 리플리케이션을 쓰는데(net.Iris.UseIrisReplication, Iris 플러그인 활성화로 켜짐), Iris 하에서
 * TagRepState 기본값(None)은 "복제 안 함"이라 서버·소유 클라(죽은 본인)에서만 태그가 보이고 시체를
 * 상호작용하려는 다른 클라에는 절대 복제되지 않아 하이라이트가 뜨지 않는다.
 *
 * 이동 차단(Controller::SetIgnoreMoveInput + 수평 속도 0)은 D1 ALyraCharacter::
 * DisableMovementAndCollision을 참고했다. Controller/CharacterMovementComponent는 서버와
 * 소유 클라(죽은 본인)에만 존재하므로 이 Ability(ServerInitiated라 서버+소유 클라에서만 실행)에서
 * 처리하는 것으로 충분하다 — 다른 클라는 CharacterMovementComponent 자체의 위치 복제로 결과만 받는다.
 *
 * DeathMontage가 지정되어 있으면 재생을 끝까지 기다린 뒤 EndAbility한다(즉시 종료하지 않음).
 * AbilitySystemComponent::PlayMontage가 세팅하는 RepAnimMontageInfo는 COND_None으로 복제되므로,
 * 서버(권위)가 재생을 시작하면 이 Ability를 실행하지 않는 다른 클라(시체를 보는 쪽)에도 그대로 보인다.
 */
UCLASS()
class A1_API UA1Ability_Death : public UCommonGameplayAbility
{
	GENERATED_BODY()

public:
	UA1Ability_Death(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	/** 사망 몽타주 재생이 끝나면(정상 종료/블렌드아웃/중단/취소 모두 동일하게) Ability를 종료한다. */
	UFUNCTION()
	void OnDeathMontageFinished();

protected:
	/** 사망 시 재생할 몽타주. 비어 있으면 몽타주 없이 즉시 종료한다. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Death")
	TObjectPtr<UAnimMontage> DeathMontage;
};
