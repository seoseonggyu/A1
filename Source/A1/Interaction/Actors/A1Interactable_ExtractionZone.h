// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Interaction/Actors/A1WorldInteractable.h"
#include "A1Interactable_ExtractionZone.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1InteractableExtractionZoneLog, Log, All);

/**
 * AA1Interactable_ExtractionZone
 *
 * 홀드형 탈출 지점. InteractEventTag(GameplayEvent.Interact.Extraction)로
 * UA1Ability_Interact_Extraction에 결과 처리를 위임한다 - AA1Interactable_Door와 동일하게
 * 상호작용 입력을 HoldDuration만큼 유지했을 때만 OnInteractAuth가 호출된다.
 *
 * 최소 버전: OnInteractAuth에서 탈출 성공을 로그로 남기고 대상 폰을 UnPossess + Destroy한다.
 * 레이드 타이머/보상/스태시 반영은 추후 별도로 붙인다.
 */
UCLASS()
class A1_API AA1Interactable_ExtractionZone : public AA1WorldInteractable
{
	GENERATED_BODY()

public:
	AA1Interactable_ExtractionZone(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void OnInteractAuth(AActor* Interactor) override;
};
