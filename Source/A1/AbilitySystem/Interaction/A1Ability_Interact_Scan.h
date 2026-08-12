// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Ability/CommonGameplayAbility.h"
#include "Engine/TimerHandle.h"
#include "A1Ability_Interact_Scan.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1AbilityInteractScanLog, Log, All);

class UPrimitiveComponent;

/**
 * UA1Ability_Interact_Scan
 *
 * 상호작용 "감지"를 담당하는 패시브 어빌리티. 소유 클라(로컬)에서만 동작한다.
 *
 * - 스폰 시(OnSpawn) 자동 활성화되어, 일정 주기로 아바타 주변을 구(Sphere) 오버랩 스캔한다.
 * - 스캔 결과 중 IA1Interactable을 구현하고 CanInteract가 true인 가장 가까운 대상을
 *   현재 대상(CurrentTarget)으로 삼아 로컬 하이라이트(CustomDepth 외곽선)를 켠다.
 * - 상호작용 입력(Input.Ability.Interact)이 들어오면(WaitInputStart), 현재 대상이 있을 때
 *   GameplayEvent.Interact를 아바타에게 보내 실제 실행 어빌리티(UA1Ability_Interact)를 발동시킨다.
 *
 * 클라이언트 전용:
 *  - NetExecutionPolicy = LocalOnly. 서버로 활성화 RPC를 보내지 않는다.
 *  - 데디케이티드 서버 사본에서는 절대 활성화되지 않는다.
 *    (원격 클라의 OnSpawn 활성화는 UCommonAbilitySystemComponent::TryActivateLocalOnlyAbilitiesOnSpawn이 수행)
 *  - 스캔·하이라이트는 순수 로컬 연출이므로 복제하지 않는다.
 *    실제 상호작용 결과 처리는 GameplayEvent로 트리거되는 UA1Ability_Interact(서버 권한)가 담당한다.
 *
 * 부여 시 InputTag = Input.Ability.Interact 로 부여해야 입력 신호(WaitInputStart)가 닿는다.
 */
UCLASS()
class A1_API UA1Ability_Interact_Scan : public UCommonGameplayAbility
{
	GENERATED_BODY()

public:
	UA1Ability_Interact_Scan(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//-----------------------------------------------------------------------------
	// UGameplayAbility 오버라이드
	//-----------------------------------------------------------------------------

	/** 서버·원격 폰에서의 활성화를 원천 차단한다. (정책 외 최종 방어선) */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	/** 주변을 구 오버랩으로 스캔해 가장 가까운 상호작용 대상을 갱신한다. 로컬 전용. (타이머로 주기 호출) */
	void ScanLocal();

	/** 현재 대상을 NewTarget으로 교체하고, 이전 대상 하이라이트는 끄고 새 대상은 켠다. */
	void SetCurrentTargetLocal(AActor* NewTarget);

	/** 대상 하이라이트(CustomDepth 외곽선)를 켜고 끈다. 로컬 전용. */
	void SetInteractableHighlightLocal(AActor* InteractableActor, bool bHighlight) const;

	/** 대상의 상호작용 프롬프트("줍기" 등) UI를 켜고 끈다. 로컬 전용. */
	void SetInteractionPromptVisibleLocal(AActor* InteractableActor, bool bVisible) const;

	/** 상호작용 입력(GameCustom1)을 1회 대기하도록 태스크를 (재)생성한다. */
	void WaitForInputStart();

	/** 입력이 들어온 순간 호출. 현재 대상이 있으면 실행 어빌리티를 트리거하고 다시 입력을 대기한다. */
	UFUNCTION()
	void OnInputStart();

private:
	/** 현재 하이라이트/트리거 대상. 비복제(로컬 전용). */
	TWeakObjectPtr<AActor> CurrentTarget;

	/** 스캔 타이머 핸들. */
	FTimerHandle ScanTimerHandle;

	/** 아바타 주변 스캔 반경(cm). */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Interact")
	float ScanRange = 200.f;

	/** 스캔 주기(초). */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Interact")
	float ScanRate = 0.1f;

	/** true면 스캔 구를 디버그 드로우한다. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Interact")
	bool bShowScanDebug = false;
};
