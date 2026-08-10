// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Ability/CommonGameplayAbility.h"
#include "A1Ability_Test.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1Ability_TestLog, Log, All);

/**
 * UA1Ability_Test
 *
 * 클라이언트 전용 동작을 확인하기 위한 테스트 어빌리티.
 *
 * - 서버(데디케이티드/리슨)에서는 절대 활성화되지 않고, 오직 NM_Client에서 로컬 컨트롤 폰에만 1회 발동한다.
 * - 발동 시점은 캐릭터 스폰 직후. 어빌리티 스펙이 소유 클라이언트로 복제되어 OnGiveAbility가 호출되거나,
 *   아바타(Pawn)가 나중에 세팅되어 OnAvatarSet이 호출되는 시점 중 먼저 조건이 갖춰지는 쪽에서 활성화된다.
 *   (ASC가 PlayerState에 있어 스펙 복제와 아바타 세팅 순서가 상황에 따라 뒤바뀌므로 두 경로를 모두 건다.)
 * - 실행 후 즉시 EndAbility로 종료하는 단발성이며, 서버로 나가는 RPC가 없도록 NetExecutionPolicy는 LocalOnly.
 */
UCLASS()
class A1_API UA1Ability_Test : public UCommonGameplayAbility
{
	GENERATED_BODY()

public:
	UA1Ability_Test(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//-----------------------------------------------------------------------------
	// UGameplayAbility 오버라이드
	//-----------------------------------------------------------------------------

	/** 서버 및 원격 폰에서의 활성화를 원천 차단하고, 스폰 1회만 허용한다. */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	/** 새 캐릭터가 세팅될 때(스폰/리스폰) 1회 발동 플래그를 초기화한다. */
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/** 클라이언트 프로세스에서 로컬 플레이어의 폰인지 판정한다. 서버(데디/리슨) 및 원격 폰은 false. */
	bool IsClientLocalContext(const FGameplayAbilityActorInfo* ActorInfo) const;

protected:
	/** 발동 시 로그/화면에 출력할 메시지 */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Test")
	FString TestMessage = TEXT("A1Ability_Test 발동 (클라이언트 전용)");

	/** 화면 디버그 메시지 출력 여부 */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Test")
	bool bPrintToScreen = true;

private:
	/** 현재 아바타에서 이미 발동했는지 여부. 아바타가 새로 세팅되면(리스폰) 초기화된다. */
	bool bActivatedForCurrentAvatar = false;
};
