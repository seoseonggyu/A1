// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Engine/NetSerialization.h"
#include "AbilitySystem/A1Ability_Equipment.h"
#include "A1Ability_Skill_AOE.generated.h"

class URangedWeaponInstance;
class UA1AbilityTask_WaitForTick;
class UA1SkillTargetingPromptWidget;
class AA1PlayerController;
class UAbilityTask_WaitConfirmCancel;

DECLARE_LOG_CATEGORY_EXTERN(A1Ability_Skill_AOELog, Log, All);

/**
 * 커서 위치에 원형 범위 피해를 발동하는 원거리(지팡이) 스킬.
 *
 * 발동 즉시 효과가 나가지 않고 2단계로 진행된다: (1) 캐스팅 시작 - AOECastMontage를 재생해
 * 다른 플레이어에게도 "이 캐릭터가 스킬을 준비 중"임을 알리며, 캐릭터는 이동은 그대로 하면서
 * 커서 방향으로만 회전하고(UA1Ability_RangedWeaponAttack의 조준 방식과 동일) 로컬 디버그 원으로
 * 조준 범위를 표시한다. UAbilityTask_WaitConfirmCancel로 확정/취소 입력을 받으며, 이 입력은
 * Input.Native.Confirm/Cancel 태그가 매핑된 키(ASC->LocalInputConfirm/Cancel)로 들어온다.
 * (2) 확정 - 비용/쿨다운을 커밋하고 서버에 장판 스폰을 요청한 뒤 AOESkillMontage(실제 스킬 발동
 * 몽타주)를 재생하며, 이 몽타주가 끝나면 어빌리티가 종료된다. 취소 시에는 아무 것도 소비하지 않고
 * 즉시 종료된다. 실제 데미지/슬로우 지속 처리는 확정 시 서버가 스폰하는 AA1SkillAOEZone이 어빌리티
 * 수명과 무관하게 전담하므로, 확정 즉시(스킬 몽타주 재생 중에도) 다음 장판 판정은 계속 진행된다.
 */
UCLASS()
class A1_API UA1Ability_Skill_AOE : public UA1Ability_Equipment
{
	GENERATED_BODY()

public:
	UA1Ability_Skill_AOE(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	void SetOrientRotationToMovement(bool bRotate);

	/** 매 프레임(WaitForTick) 호출. 조준 회전과 로컬 인디케이터 드로우를 처리한다. */
	UFUNCTION()
	void OnTargetingTick(float DeltaTime);

	/** 소유 클라 전용. 커서 위치로 회전을 갱신하고 인디케이터를 그린다. */
	void UpdateTargetingLocal(float DeltaTime, AA1PlayerController* PlayerController);

	void StopAimTickLocal();

	/** UAbilityTask_WaitConfirmCancel::OnConfirm. Input.Native.Confirm 입력(ASC->LocalInputConfirm)으로 트리거된다. */
	UFUNCTION()
	void OnConfirmInput();

	/** UAbilityTask_WaitConfirmCancel::OnCancel. Input.Native.Cancel 입력(ASC->LocalInputCancel)으로 트리거된다. */
	UFUNCTION()
	void OnCancelInput();

	/** 소유 클라 → 서버로 최신 조준 방향을 전달한다. (UA1Ability_RangedWeaponAttack과 동일한 이유) */
	UFUNCTION(Server, Unreliable)
	void UpdateAimDirectionServer(FVector_NetQuantizeNormal AimDirection);

	void SyncAimDirectionToServerLocal();

	/** 확정. 조준을 정지하고 비용/쿨다운을 커밋해 서버에 장판 스폰을 요청한 뒤 AOESkillMontage를 재생한다. */
	void ConfirmTargetLocal();

	/** 취소. 아무 것도 소비/적용하지 않고 어빌리티를 취소 종료한다. */
	void CancelTargetLocal();

	/** 소유 클라 → 서버. 확정된 커서 월드 위치에 장판을 스폰하도록 요청한다. */
	UFUNCTION(Server, Reliable)
	void ConfirmAOEServer(FVector_NetQuantize TargetLocation);

	/** 서버 전용. AA1SkillAOEZone을 스폰하고 무기 인스턴스의 수치를 주입한다. */
	void SpawnAOEZoneAuth(const FVector& TargetLocation);

	/** 캐스팅 시작을 알리는 몽타주 재생(다른 플레이어에게도 복제되어 보인다). 몽타주가 없으면 아무 것도 하지 않는다. */
	void PlayCastMontage();

	/** 확정 시 실제 스킬 몽타주를 재생한다. 몽타주가 없으면 즉시 OnSkillMontageFinished로 넘어간다. */
	void PlaySkillMontage();

	/** AOESkillMontage 재생 종료(정상/블렌드아웃/중단/취소 모두) 시 호출되어 어빌리티를 종료한다. */
	UFUNCTION()
	void OnSkillMontageFinished();

	URangedWeaponInstance* GetRangedWeaponInstance() const;

	/** 로컬 전용. 인디케이터 원을 커서 위치에 한 프레임만 그린다(매 틱 다시 그려 따라다니는 것처럼 보인다). */
	void DrawTargetingIndicatorLocal(const FVector& Center) const;

	void ShowTargetingPromptLocal() const;
	void HideTargetingPromptLocal() const;
	UA1SkillTargetingPromptWidget* FindTargetingPromptWidgetLocal() const;

private:
	/** 소유 클라에서 매 프레임 계산한 최신 조준 방향(정규화). */
	FVector LocalAimDirection = FVector::ForwardVector;

	/** 서버가 UpdateAimDirectionServer로 전달받은 최신 조준 방향. 원격 클라 소유 캐릭터의 회전에 쓰인다. */
	FVector ServerAimDirection = FVector::ForwardVector;

	/** 소유 클라에서 매 프레임 갱신되는, 확정 시 서버로 보낼 커서 월드 위치. */
	FVector CachedTargetLocation = FVector::ZeroVector;

	/** 확정 처리가 중복 실행되지 않도록 막는다(OnConfirmInput 재호출 방지). */
	bool bTargetConfirmed = false;

	static constexpr float AimSyncInterval = 0.1f;
	FTimerHandle AimSyncTimerHandle;

	/** 조준 회전·인디케이터를 매 프레임 돌리는 태스크. EndAbility에서 정리한다. */
	UPROPERTY()
	TObjectPtr<UA1AbilityTask_WaitForTick> AimTickTask;

	/** 확정/취소 입력을 받는 태스크. EndAbility에서 정리한다. */
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitConfirmCancel> ConfirmCancelTask;

protected:
	/** 조준 회전 보간 속도(FMath::RInterpTo의 InterpSpeed). */
	UPROPERTY(EditDefaultsOnly, Category = "A1|AOE Skill")
	float AimRotationInterpSpeed = 20.f;

	/** 조준 중 표시할 로컬 디버그 원 색상. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|AOE Skill|Debug")
	FColor IndicatorColor = FColor::Yellow;
};
