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
 * 드롭할 아이템은 UI(예: UA1ItemDropWidget)가 ItemId를 실어 GameplayEvent로 트리거할 때
 * 결정된다(UItemInstance*는 네트워크 주소 지정이 안 되는 오브젝트라 클라 → 서버로 안정적으로
 * 복제되지 않으므로 ID로만 넘긴다). 스폰 지점 계산은 별도 함수로 분리해 둔다.
 *
 * 실행 위치:
 *  - ActivationPolicy = Manual, AbilityTriggers = GameplayEvent.DropItem 로 발동되는
 *    이벤트 트리거 어빌리티. 발동 측(UI)이 소유 클라에서 ASC->HandleGameplayEvent를
 *    직접 호출해야 한다(입력 바인딩 없음).
 *  - NetExecutionPolicy = LocalPredicted(베이스 기본값). 소유 클라에서 이벤트를 보내면
 *    TriggerEventData가 서버에도 함께 전달되지만, 실제 검증·액터 스폰은 서버(Authority)
 *    에서만 수행한다. (스폰된 픽업 액터는 bReplicates=true라 클라로 복제된다)
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
	 * TriggerEventData->EventMagnitude(ItemId)로 지정된 특정 아이템을 인벤토리에서 제거하고
	 * 픽업 액터를 스폰한다. 서버(Authority) 전용.
	 */
	void DropItemAuth(UItemInstance* ItemToDrop);

	/**
	 * ItemToDrop이 실제로 이 Inventory 소속인지 확인하고, 장착 중이면 해제할 슬롯 태그를,
	 * 표시 메시(StaticMesh 우선, 없으면 SkeletalMesh)·배치 스케일(원본 장비의 AttachTransform)을
	 * 채운다. 서버(Authority) 전용. Item이 이 Inventory 소속이 아니면 false.
	 */
	bool ResolveDropContextAuth(UItemInstance* ItemToDrop, UInventoryComponent* Inventory, FGameplayTag& OutUnequipSlotTag, UStaticMesh*& OutStaticMesh, USkeletalMesh*& OutSkeletalMesh, FVector& OutDisplayScale) const;

	/** 아바타 앞 바닥에 드롭(픽업) 액터를 스폰하고 아이템 정보·표시 메시·스케일을 실어준다. 서버(Authority) 전용. */
	void SpawnDropActorAuth(const UItemDefinition* ItemDefinition, int32 ItemCount, UStaticMesh* DisplayStaticMesh, USkeletalMesh* DisplaySkeletalMesh, const FVector& DisplayScale);

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
