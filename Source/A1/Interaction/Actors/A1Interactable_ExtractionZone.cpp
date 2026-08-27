// Copyright Epic Games, Inc. All Rights Reserved.

#include "A1Interactable_ExtractionZone.h"

#include "A1GameplayTags.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Interactable_ExtractionZone)

DEFINE_LOG_CATEGORY(A1InteractableExtractionZoneLog);

AA1Interactable_ExtractionZone::AA1Interactable_ExtractionZone(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InteractionTitle = NSLOCTEXT("A1Interaction", "ExtractionZone", "탈출하기");
	// 사용 중인 값: WorldInteractable 기본=1, Door=2, Pickup=3, Corpse=4, Character=5, ExtractionZone=6.
	HighlightStencil = 6;
	// UA1Ability_Interact가 이 태그로 UA1Ability_Interact_Extraction(홀드 판정)에 결과 처리를 위임한다.
	InteractEventTag = A1GameplayTags::GameplayEvent_Interact_Extraction;
}

void AA1Interactable_ExtractionZone::OnInteractAuth(AActor* Interactor)
{
	Super::OnInteractAuth(Interactor);

	if (HasAuthority() == false)
	{
		return;
	}

	UE_LOG(A1InteractableExtractionZoneLog, Log, TEXT("탈출 성공: %s"), *GetNameSafe(Interactor));

	// TODO: 탈출 처리
}
