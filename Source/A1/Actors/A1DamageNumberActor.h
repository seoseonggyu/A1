// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "Coro.h"
#include "A1DamageNumberActor.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1DamageNumberActorLog, Log, All);

class UWidgetComponent;

/**
 * AA1DamageNumberActor
 *
 * 피격 데미지 숫자를 화면에 잠깐 띄우는 연출 전용 액터.
 * UA1GameplayCueNotify_DamageNumber가 클라이언트에서만 스폰하며, 복제되지 않는다.
 * InitializeDamageNumber로 위젯에 데미지 값을 채운 뒤 코루틴으로
 * "오버슈트 팝업 → 유지 중 위로 떠오름 + 흔들림 감쇠 + 페이드아웃" 연출을 재생하고
 * 스스로 파괴된다. Widget Class는 BP 자식에서 UA1DamageNumberWidget 파생으로 지정한다.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class A1_API AA1DamageNumberActor : public AActor
{
	GENERATED_BODY()

public:
	AA1DamageNumberActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** 데미지 값을 위젯에 채우고 팝업 연출을 시작한다. */
	void InitializeDamageNumber(float DamageAmount);

protected:
	/** 데미지 숫자 위젯. 화면 스페이스로 렌더링된다. Widget Class는 BP에서 지정. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "A1|DamageNumber")
	TObjectPtr<UWidgetComponent> DamageWidgetComponent;

	//-----------------------------------------------------------------------------
	// 팝업(오버슈트) 단계
	//-----------------------------------------------------------------------------

	/** 팝업 시작 스케일. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|DamageNumber")
	float StartScale = 0.3f;

	/** 목표 스케일 대비 얼마나 더 크게 튀어오르는지의 배율. 1.0이면 오버슈트 없음. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|DamageNumber")
	float OvershootMultiplier = 1.3f;

	/** Start → Overshoot 구간에 걸리는 프레임 수. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|DamageNumber")
	int32 PopFrameCount = 5;

	/** Overshoot → Target으로 눌러앉는 구간에 걸리는 프레임 수. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|DamageNumber")
	int32 SettleFrameCount = 4;

	//-----------------------------------------------------------------------------
	// 유지(플로트/흔들림/페이드) 단계
	//-----------------------------------------------------------------------------

	/** 표시를 유지하는 시간(초, 팝업 단계 종료 후부터). */
	UPROPERTY(EditDefaultsOnly, Category = "A1|DamageNumber")
	float HoldDuration = 0.8f;

	/** 유지 구간 시작 시점의 흔들림 폭(화면 픽셀). 시간이 지날수록 0으로 감쇠한다. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|DamageNumber")
	float ShakeAmplitude = 2.f;

	/** 흔들림이 매 프레임 랜덤 목표치를 얼마나 빨리 따라가는지(0~1). 낮을수록 부드럽고, 1이면 프레임마다 순간이동하듯 튄다. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|DamageNumber", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ShakeSmoothing = 0.35f;

	/** 유지 구간 동안 위로 떠오르는 총 거리(화면 픽셀). */
	UPROPERTY(EditDefaultsOnly, Category = "A1|DamageNumber")
	float FloatDistance = 60.f;

	/** 유지 구간 진행률(0~1) 중 이 시점부터 페이드아웃을 시작한다. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|DamageNumber", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FadeOutStartRatio = 0.6f;

private:
	TCoroTask<void> PlayPopAnimationCoroutine(float TargetScale);
};
