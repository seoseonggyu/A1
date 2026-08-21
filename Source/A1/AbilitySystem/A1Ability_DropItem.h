// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Ability/CommonGameplayAbility.h"
#include "A1Ability_DropItem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1AbilityDropItemLog, Log, All);

class AA1WorldInteractable;
class UItemDefinition;
class UItemInstance;
class UInventoryComponent;
class UStaticMesh;
class USkeletalMesh;

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
	/**
	 * 드롭할 아이템을 결정하고(장착/인벤토리), 정의·수량을 확보한 뒤 인벤토리에서 제거하고
	 * 픽업 액터를 스폰한다. 서버(Authority) 전용.
	 */
	void DropItemAuth();

	/**
	 * bDropEquipped에 따라 드롭할 원본 아이템을 고른다. 장착 아이템이면 해제할 슬롯 태그도 채운다.
	 * 아이템이 정해지면 표시 메시(StaticMesh 우선, 없으면 SkeletalMesh)와 배치 스케일(원본 장비의
	 * AttachTransform)을 함께 찾아 채운다. 서버(Authority) 전용. (없으면 nullptr)
	 */
	UItemInstance* SelectItemToDropAuth(APawn* Avatar, UInventoryComponent* Inventory, FGameplayTag& OutUnequipSlotTag, UStaticMesh*& OutStaticMesh, USkeletalMesh*& OutSkeletalMesh, FVector& OutDisplayScale) const;

	/** 아바타 앞 바닥에 드롭(픽업) 액터를 스폰하고 아이템 정보·표시 메시·스케일을 실어준다. 서버(Authority) 전용. */
	void SpawnDropActorAuth(const UItemDefinition* ItemDefinition, int32 ItemCount, UStaticMesh* DisplayStaticMesh, USkeletalMesh* DisplaySkeletalMesh, const FVector& DisplayScale);

	/** 아바타 기준으로 드롭 액터를 놓을 위치·회전을 계산한다. (바닥으로 라인 트레이스) */
	FTransform ComputeDropTransform(const AActor* Avatar) const;

private:
	/** 스폰할 드롭(픽업) 액터 클래스. BP에서 메시가 지정된 픽업으로 지정한다. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Drop")
	TSubclassOf<AA1WorldInteractable> DropActorClass;

	/** true면 손에 든(활성) 장착 아이템을, false면 인벤토리(비장착) 아이템을 드롭한다. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Drop")
	bool bDropEquipped = false;

	/** 아바타 정면으로 얼마나 떨어뜨릴지(cm). */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Drop")
	float DropForwardDistance = 100.f;

	/** 바닥 탐지 라인 트레이스 길이(cm). 스폰 지점을 지면에 붙인다. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|Drop")
	float GroundTraceDistance = 500.f;
};
