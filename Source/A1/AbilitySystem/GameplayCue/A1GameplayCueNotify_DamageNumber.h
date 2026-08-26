// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayCueNotify_Static.h"
#include "A1GameplayCueNotify_DamageNumber.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1GameplayCueNotify_DamageNumberLog, Log, All);

class AA1DamageNumberActor;

/**
 * UA1GameplayCueNotify_DamageNumber
 *
 * GameplayCue.Character.DamageTaken 태그로 실행되는 즉시성(Static) Cue.
 * 피격 대상(MyTarget) 위, 살짝 랜덤한 위치에 DamageNumberActorClass를 스폰해 데미지 숫자를 띄운다.
 * 데디케이티드 서버에는 표시할 뷰포트가 없으므로 스폰을 건너뛴다.
 * DamageNumberActorClass는 BP 자식 에셋에서 반드시 지정해야 한다.
 */
UCLASS(Blueprintable)
class A1_API UA1GameplayCueNotify_DamageNumber : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

protected:
	/** 스폰할 데미지 숫자 액터 클래스. BP에서 지정 필수(위젯 클래스가 붙은 BP 자식). */
	UPROPERTY(EditDefaultsOnly, Category = "A1|DamageNumber")
	TSubclassOf<AA1DamageNumberActor> DamageNumberActorClass;

	/** 대상 위로 띄우는 기본 높이(cm). */
	UPROPERTY(EditDefaultsOnly, Category = "A1|DamageNumber")
	float HeightOffset = 100.f;

	/** 수평 랜덤 오프셋 반경(cm). */
	UPROPERTY(EditDefaultsOnly, Category = "A1|DamageNumber")
	float RandomOffsetRadius = 40.f;
};
