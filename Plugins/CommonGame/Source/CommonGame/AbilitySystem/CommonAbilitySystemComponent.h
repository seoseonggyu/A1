// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Ability/CommonAbilityTypes.h"
#include "CommonAbilitySystemComponent.generated.h"

class UCommonGameplayAbility;

/**
 * CommonGame의 AbilitySystemComponent
 *
 * GameplayTag 기반 입력-Ability 연동을 지원합니다.
 * DynamicSpecSourceTags로 등록된 InputTag를 매칭하여 Ability를 활성화합니다.
 */
UCLASS()
class COMMONGAME_API UCommonAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UCommonAbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//-----------------------------------------------------------------------------
	// UAbilitySystemComponent 오버라이드
	//-----------------------------------------------------------------------------

	virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;
	virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;

	//-----------------------------------------------------------------------------
	// 초기화
	//-----------------------------------------------------------------------------

	/**
	 * ASC와 AttributeSet의 네트워크 복제가 완료된 시점에 호출됩니다 (클라이언트에서만 호출)
	 *
	 * 클라이언트에서 ASC, ActorInfo, AttributeSet이 모두 복제된 것이 보장됩니다.
	 */
	virtual void PostNetInit();

	/** PostNetInit이 호출되었는지 반환합니다 */
	bool IsPostNetInitialized() const { return bPostNetInitialized; }

	//-----------------------------------------------------------------------------
	// 입력-Ability 연동
	//-----------------------------------------------------------------------------

	/**
	 * 입력 태그가 눌렸음을 알립니다
	 *
	 * DynamicSpecSourceTags에 해당 태그가 있는 Ability를 찾아
	 * InputPressedSpecHandles와 InputHeldSpecHandles에 추가합니다
	 * @param InputTag 눌린 입력 태그
	 */
	void AbilityInputTagPressed(const FGameplayTag& InputTag);

	/**
	 * 입력 태그가 해제되었음을 알립니다
	 *
	 * DynamicSpecSourceTags에 해당 태그가 있는 Ability를 찾아
	 * InputReleasedSpecHandles에 추가하고 InputHeldSpecHandles에서 제거합니다
	 * @param InputTag 해제된 입력 태그
	 */
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

	/**
	 * 입력 상태를 처리하여 Ability를 활성화/종료합니다
	 *
	 * ActivationPolicy에 따라 적절한 시점에 Ability를 활성화합니다
	 * 매 프레임 호출되어야 합니다 (PostProcessInput)
	 * @param DeltaTime 프레임 델타 시간
	 * @param bGamePaused 게임 일시정지 여부
	 */
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);

	/** 입력 상태를 초기화합니다 */
	void ClearAbilityInput();

	//-----------------------------------------------------------------------------
	// ActivationGroup 관리
	//-----------------------------------------------------------------------------

	/** 해당 그룹의 활성화가 차단되었는지 확인합니다 */
	bool IsActivationGroupBlocked(ECommonAbilityActivationGroup Group) const;

	/** Ability를 활성화 그룹에 추가합니다 */
	void AddAbilityToActivationGroup(ECommonAbilityActivationGroup Group, UCommonGameplayAbility* Ability);

	/** Ability를 활성화 그룹에서 제거합니다 */
	void RemoveAbilityFromActivationGroup(ECommonAbilityActivationGroup Group, UCommonGameplayAbility* Ability);

	/** 해당 그룹의 모든 Ability를 취소합니다 */
	void CancelActivationGroupAbilities(ECommonAbilityActivationGroup Group, UCommonGameplayAbility* IgnoreAbility);

protected:
	/** 이번 프레임에 Press된 Ability 핸들들 */
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	/** 이번 프레임에 Release된 Ability 핸들들 */
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	/** 현재 Hold 중인 Ability 핸들들 */
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;

	/** 활성화 그룹별 Ability 카운트 */
	int32 ActivationGroupCounts[static_cast<uint8>(ECommonAbilityActivationGroup::MAX)];

	/** PostNetInit 호출 여부 */
	bool bPostNetInitialized = false;
};