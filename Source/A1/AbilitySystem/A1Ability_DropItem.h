// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Ability/CommonGameplayAbility.h"
#include "A1Ability_DropItem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1AbilityDropItemLog, Log, All);

class AA1WorldInteractable;

/**
 * UA1Ability_DropItem
 *
 * 아바타 앞 바닥에 "드롭(픽업) 액터"를 하나 스폰하는 어빌리티.
 * 지금은 테스트 목적으로 입력 키에 바인딩해 발동하며, 이후 인벤토리에서 아이템을
 * 버리는 행위로 확장할 수 있도록 스폰 지점을 별도 함수로 분리해 둔다.
 *
 * 실행 위치:
 *  - 입력(Input.Ability.DropItem)으로 발동되는 OnInputTriggered 어빌리티.
 *  - NetExecutionPolicy = LocalPredicted(베이스 기본값). 소유 클라에서 발동되면 서버로
 *    활성화 RPC가 전달되지만, 실제 액터 스폰은 서버(Authority)에서만 수행한다.
 *    (스폰된 픽업 액터는 bReplicates=true라 클라로 복제된다)
 *
 * 부여 시 InputTag = Input.Ability.DropItem 로 부여해야 입력 신호가 닿는다.
 */
UCLASS()
class A1_API UA1Ability_DropItem : public UCommonGameplayAbility
{
	GENERATED_BODY()

public:
	UA1Ability_DropItem(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//-----------------------------------------------------------------------------
	// UGameplayAbility 오버라이드
	//-----------------------------------------------------------------------------

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	/** 아바타 앞 바닥에 드롭 액터를 스폰한다. 서버(Authority) 전용. */
	void SpawnDropActorAuth();

	/** 아바타 기준으로 드롭 액터를 놓을 위치·회전을 계산한다. (바닥으로 라인 트레이스) */
	FTransform ComputeDropTransform(const AActor* Avatar) const;

private:
	/** 스폰할 드롭(픽업) 액터 클래스. BP에서 메시가 지정된 픽업으로 지정한다. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Drop")
	TSubclassOf<AA1WorldInteractable> DropActorClass;

	/** 아바타 정면으로 얼마나 떨어뜨릴지(cm). */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Drop")
	float DropForwardDistance = 100.f;

	/** 바닥 탐지 라인 트레이스 길이(cm). 스폰 지점을 지면에 붙인다. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Drop")
	float GroundTraceDistance = 500.f;
};
