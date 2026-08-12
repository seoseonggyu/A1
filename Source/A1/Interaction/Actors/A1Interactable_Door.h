// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Interaction/Actors/A1WorldInteractable.h"
#include "A1Interactable_Door.generated.h"

/**
 * AA1Interactable_Door
 *
 * 홀드형 상호작용 문 예시. InteractEventTag(GameplayEvent.Interact.Door)로
 * UA1Ability_Interact_Door에 결과 처리를 위임한다 - 실제 문 열기(OnInteractAuth)는
 * 상호작용 입력을 HoldDuration만큼 유지했을 때만 호출된다.
 *
 * 소모형이 아니므로(bConsumeOnUse=false) 여러 번 열고 닫는 연출은 K2_OnInteractAuth에서
 * BP로 확장한다. (기본은 로그만 남기는 Super::OnInteractAuth 그대로 사용)
 */
UCLASS()
class A1_API AA1Interactable_Door : public AA1WorldInteractable
{
	GENERATED_BODY()

public:
	AA1Interactable_Door(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
