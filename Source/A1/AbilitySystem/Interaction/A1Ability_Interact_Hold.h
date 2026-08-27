// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Ability/CommonGameplayAbility.h"
#include "Coro.h"
#include "A1Ability_Interact_Hold.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1AbilityInteractHoldLog, Log, All);

class UAnimMontage;
class UA1InteractionHoldWidget;

/**
 * UA1Ability_Interact_Hold
 *
 * 홀드형 상호작용 어빌리티 공통 베이스(추상). GameplayEvent로 트리거되어 HoldDuration만큼
 * 입력을 유지하면 결과 처리, 도중에 입력을 떼면 취소되는 흐름(UA1Ability_Interact_Extraction,
 * UA1Ability_Interact_LootContainer가 파생)을 한 곳에 모았다.
 *
 * 파생 클래스는 생성자에서 SetAssetTags/AbilityTriggers(트리거 GameplayEvent 태그)와
 * HoldDuration을 지정하고, 홀드를 완주했을 때 할 일을 OnHoldCompletedAuth(서버 결과 처리)
 * 그리고/또는 OnHoldCompletedLocal(소유 클라 UI 등)로 오버라이드하면 된다.
 *
 * 입력 유지 감지 전제조건 (에셋 설정 필요):
 *  - 이 어빌리티가 UA1Ability_Interact_Scan과 "같은" InputTag(Input.Ability.Interact)로 부여되어야
 *    UAbilityTask_WaitInputRelease가 릴리즈 신호를 받을 수 있다.
 *
 * 실행 위치:
 *  - NetExecutionPolicy = ServerInitiated. 서버가 로컬로 발동을 결정(HandleGameplayEvent)하면
 *    그 활성화가 소유 클라에도 복제되어, 서버·소유 클라가 각자 자기 쪽에서 홀드/몽타주/UI를 진행한다.
 *  - OnHoldCompletedAuth는 서버 인스턴스에서만, OnHoldCompletedLocal은 소유 클라 인스턴스에서만 호출된다.
 *
 * 홀드 진행 UI: HUD 어딘가에 미리 배치되어 항상 존재하는 UA1InteractionHoldWidget 인스턴스를
 * UCommonPrimaryGameLayout::FindWidgetOfType로 찾아 StartHold/StopHold만 호출한다.
 * (Push/Pop 레이어 스택 방식은 쓰지 않는다 - 홀드 중 반복적으로 입력 모드를 재계산시켜
 * 입력이 흔들리는 부작용이 있었다.)
 */
UCLASS(Abstract)
class A1_API UA1Ability_Interact_Hold : public UCommonGameplayAbility
{
	GENERATED_BODY()

public:
	UA1Ability_Interact_Hold(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/** 홀드를 완주했을 때 서버 인스턴스에서만 호출된다(예: 대상 IA1Interactable::OnInteractAuth 호출). */
	virtual void OnHoldCompletedAuth(AActor* Interactor, AActor* Target) {}

	/** 홀드를 완주했을 때 소유 클라 인스턴스에서만 호출된다(예: 루팅 창 표시). */
	virtual void OnHoldCompletedLocal(AActor* Interactor, AActor* Target) {}

private:
	/** HoldDuration만큼 대기한 뒤, 그동안 입력이 유지됐으면 OnHoldCompletedAuth/Local을 호출한다. */
	TCoroTask<void> RunHoldCoroutine();

	/** 상호작용 입력을 떼면 호출된다. 아직 홀드가 끝나지 않았다면 취소한다. */
	UFUNCTION()
	void OnInputReleased(float TimeHeld);

	/** 소유 클라 전용: 홀드 진행 UI를 화면에 띄우고 재생을 시작한다. */
	void ShowHoldWidgetLocal();

	/** 소유 클라 전용: 홀드 진행 UI를 제거한다(완료/취소 무관). */
	void HideHoldWidgetLocal();

	/**
	 * 소유 클라 전용: HUD 안에 상시 배치된 UA1InteractionHoldWidget 인스턴스를 찾는다.
	 * HUD는 UI.Layer.Game에 Push된 위젯(W_GameLayout 등)의 자식이라 WidgetTree만으로는
	 * 못 찾고, 그 레이어의 "현재 활성 위젯" 트리 안에서 찾아야 한다.
	 */
	UA1InteractionHoldWidget* FindHoldWidgetLocal() const;

protected:
	/** 입력 유지 시간(초). 파생 클래스 생성자에서 지정한다. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Interact")
	float HoldDuration = 2.f;

	/**
	 * 홀드 중 재생할 모션(선택). 미지정이면 몽타주 없이 홀드만 판정한다.
	 * HoldDuration이 몽타주 길이보다 길 수 있으므로, 몽타주 자체에 루프 섹션(Next Section을
	 * 자기 자신으로 지정)을 만들어 재생 시간과 무관하게 반복되도록 구성한다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Interact")
	TObjectPtr<UAnimMontage> HoldMontage;

	/** 홀드 대상 액터. */
	TWeakObjectPtr<AActor> TargetActor;

private:
	/** 홀드 완료 전에 입력이 풀렸는지 여부. */
	bool bInputReleased = false;

	/** RunHoldCoroutine의 소유권. 어빌리티 인스턴스가 살아있는 동안 유지되어야 한다. */
	TCoroTask<void> PendingHoldTask;

	/**
	 * ShowHoldWidgetLocal을 실제로 호출했는지 여부. 대상 검증 실패 등으로 위젯을 띄우기 전에
	 * EndAbility가 호출될 수 있어, 불필요한 FindWidgetOfType 탐색을 피하려고 둔다.
	 */
	bool bHoldWidgetVisible = false;
};
