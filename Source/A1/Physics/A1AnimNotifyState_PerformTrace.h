#pragma once

#include "A1Define.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Engine/EngineTypes.h"
#include "A1AnimNotifyState_PerformTrace.generated.h"

class UBoxComponent;

DECLARE_LOG_CATEGORY_EXTERN(A1AnimNotifyState_PerformTraceLog, Log, All);

//-----------------------------------------------------------------------------
// FA1TraceParams
//-----------------------------------------------------------------------------

/** PerformTrace가 사용할 트레이스 세부 파라미터 */
USTRUCT(BlueprintType)
struct FA1TraceParams
{
	GENERATED_BODY()

public:
	/**
	 * 무기 액터에서 찾을 CollisionBox의 컴포넌트 태그.
	 * BP 무기 액터의 BoxComponent에 이 태그를 등록하면 해당 박스를 셰이프로 사용한다.
	 * (비워 두면 액터의 첫 번째 BoxComponent를 사용)
	 */
	UPROPERTY(EditAnywhere)
	FName CollisionComponentTag = TEXT("TraceBox");

	/**
	 * 오버랩으로 검출할 오브젝트 타입 목록.
	 * 비워 두면 Pawn을 검출한다.
	 */
	UPROPERTY(EditAnywhere)
	TArray<TEnumAsByte<EObjectTypeQuery>> DetectionObjectTypes;
};

//-----------------------------------------------------------------------------
// FA1TraceDebugParams
//-----------------------------------------------------------------------------

/** 트레이스 디버그 드로잉 파라미터 */
USTRUCT(BlueprintType)
struct FA1TraceDebugParams
{
	GENERATED_BODY()

public:
	/** 검사 박스를 디버그로 그린다. */
	UPROPERTY(EditAnywhere)
	bool bDrawDebugShape = false;

	/** 히트가 없을 때 색상 */
	UPROPERTY(EditAnywhere)
	FColor TraceColor = FColor::Red;

	/** 히트가 있을 때 색상 */
	UPROPERTY(EditAnywhere)
	FColor HitColor = FColor::Green;

	/** 디버그 박스 표시 시간(초) */
	UPROPERTY(EditAnywhere)
	float DrawDuration = 1.f;
};

//-----------------------------------------------------------------------------
// UA1AnimNotifyState_PerformTrace
//-----------------------------------------------------------------------------

/**
 * 공격 몽타주 구간 동안 무기 액터의 CollisionBox 위치에서 오버랩으로 충돌을 검사하는 AnimNotifyState.
 *
 * - Begin : 무기 액터와 CollisionBox를 찾아 캐시하고, 이미 맞은 대상 목록을 비운다.
 * - Tick / End : 박스의 현재 위치에서 오버랩을 검사하고, 새로 맞은 대상에 대해 EventTag 게임플레이 이벤트를 보낸다.
 *
 * 블로킹(Block) 없이 오버랩만 검사하므로 스윕/서브스텝이 필요 없다.
 * 박스는 셰이프(크기·트랜스폼) 참조로만 쓰이며, 박스 자체의 콜리전 On/Off와 무관하게 동작한다.
 * 실제 데미지 처리는 EventTag를 수신하는 GameplayAbility(예: MeleeWeaponAttack) 쪽에서 담당한다.
 */
UCLASS()
class UA1AnimNotifyState_PerformTrace : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UA1AnimNotifyState_PerformTrace(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	void PerformTrace(USkeletalMeshComponent* MeshComponent);

	/** 무기 액터에서 사용할 CollisionBox를 찾는다. (태그 우선, 없으면 첫 번째 BoxComponent) */
	UBoxComponent* FindCollisionBox(AActor* Actor) const;

public:
	/** 이 NetRole에서만 트레이스를 실행한다. (기본: 서버 전용) */
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ENetRole> ExecuteNetRole = ROLE_Authority;

	/** 히트 시 소유 캐릭터에게 보낼 게임플레이 이벤트 태그 (예: GameplayEvent.Trace) */
	UPROPERTY(EditAnywhere, meta = (Categories = "GameplayEvent"))
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere)
	FA1TraceParams TraceParams;

	UPROPERTY(EditAnywhere)
	FA1TraceDebugParams TraceDebugParams;

private:
	/** 스폰된 무기 액터 (EquipmentComponent에서 조회) */
	UPROPERTY()
	TWeakObjectPtr<AActor> WeaponActor;

	/** 무기 액터의 트레이스용 CollisionBox */
	UPROPERTY()
	TWeakObjectPtr<UBoxComponent> CollisionBox;

	/** 이번 구간에서 이미 맞은 대상 (중복 히트 방지) */
	UPROPERTY()
	TSet<TWeakObjectPtr<AActor>> HitActors;
};
