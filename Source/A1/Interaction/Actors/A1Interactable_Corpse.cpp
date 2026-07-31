#include "A1Interactable_Corpse.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Interactable_Corpse)

AA1Interactable_Corpse::AA1Interactable_Corpse(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InteractionTitle = NSLOCTEXT("A1Interaction", "Corpse", "루팅");
	HighlightStencil = 4;
	InteractEventTag = FGameplayTag();
	// 반복 루팅(창 열기) 허용. 실제 루팅 UI/전리품은 K2_OnInteractAuth에서 확장.
	bConsumeOnUse = false;
}
