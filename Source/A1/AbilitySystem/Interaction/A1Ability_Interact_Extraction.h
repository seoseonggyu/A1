// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystem/Interaction/A1Ability_Interact_Hold.h"
#include "A1Ability_Interact_Extraction.generated.h"

/**
 * UA1Ability_Interact_Extraction
 *
 * 탈출 지점 전용 상호작용 어빌리티. UA1Ability_Interact가 대상의 InteractEventTag
 * (GameplayEvent.Interact.Extraction)로 서버에서만 트리거한다(TriggerEventData->Target이 탈출 지점 액터).
 *
 * 홀드 판정 자체는 UA1Ability_Interact_Hold가 담당하고, 이 클래스는 홀드를 완주했을 때
 * 서버 인스턴스에서 대상의 OnInteractAuth를 호출하는 결과 처리만 담당한다.
 */
UCLASS()
class A1_API UA1Ability_Interact_Extraction : public UA1Ability_Interact_Hold
{
	GENERATED_BODY()

public:
	UA1Ability_Interact_Extraction(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	/** 홀드를 완주한 서버 인스턴스에서만 호출된다. 대상의 OnInteractAuth로 탈출 결과 처리를 위임한다. */
	virtual void OnHoldCompletedAuth(AActor* Interactor, AActor* Target) override;
};
