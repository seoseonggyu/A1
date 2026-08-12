// Copyright Epic Games, Inc. All Rights Reserved.

#include "A1Interactable_Door.h"

#include "A1GameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Interactable_Door)

AA1Interactable_Door::AA1Interactable_Door(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InteractionTitle = NSLOCTEXT("A1Interaction", "Door", "문 열기");
	// 사용 중인 값: WorldInteractable 기본=1, Door=2, Pickup=3, Corpse=4, Character=5.
	HighlightStencil = 2;
	// UA1Ability_Interact가 이 태그로 UA1Ability_Interact_Door(홀드 판정)에 결과 처리를 위임한다.
	InteractEventTag = A1GameplayTags::GameplayEvent_Interact_Door;
}
