#include "A1Interactable_Pickup.h"

#include "A1GameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Interactable_Pickup)

AA1Interactable_Pickup::AA1Interactable_Pickup(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InteractionTitle = NSLOCTEXT("A1Interaction", "Pickup", "줍기");
	HighlightStencil = 3;
	// UA1Ability_Interact가 이 태그로 UA1Ability_Interact_Pickup(줍기 모션)에 결과 처리를 위임한다.
	InteractEventTag = A1GameplayTags::GameplayEvent_Interact_Pickup;
	// 1회 획득 후 제거되므로 소모형으로 취급.
	bConsumeOnUse = true;
}

void AA1Interactable_Pickup::OnInteractAuth(AActor* Interactor)
{
	if (HasAuthority() == false)
		return;

	// Super가 bIsUsed=true 처리 + 로그 + K2_OnInteractAuth(인벤토리 추가 확장 지점) 호출.
	Super::OnInteractAuth(Interactor);

	// TODO(inventory): Interactor의 InventoryComponent에 아이템 추가 (a1-item-equipment).
	//                   지금은 프레임워크 검증을 위해 획득 후 액터를 제거만 한다.
	Destroy();
}
