// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Inventory/A1ItemDropWidget.h"
#include "UI/Inventory/A1ItemDragDrop.h"

#include "A1GameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Inventory/ItemInstance.h"
#include "GameFramework/Pawn.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1ItemDropWidget)

DEFINE_LOG_CATEGORY(A1ItemDropWidgetLog);

bool UA1ItemDropWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	const UA1ItemDragDrop* DragOp = Cast<UA1ItemDragDrop>(InOperation);
	if (!DragOp || !DragOp->ItemInstance)
	{
		UE_LOG(A1ItemDropWidgetLog, Warning, TEXT("NativeOnDrop: 유효하지 않은 드래그 페이로드입니다."));
		return false;
	}

	APawn* OwningPawn = GetOwningPlayerPawn();
	UAbilitySystemComponent* ASC = OwningPawn ? UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningPawn) : nullptr;
	if (!ASC)
	{
		UE_LOG(A1ItemDropWidgetLog, Warning, TEXT("NativeOnDrop: AbilitySystemComponent를 찾을 수 없습니다."));
		return false;
	}

	// UA1Ability_DropItem을 GameplayEvent로 발동한다. UItemInstance*는 네트워크 주소 지정이 안 되는
	// 오브젝트라 서버로 안정적으로 안 넘어가므로, ItemId(EventMagnitude)만 실어 보낸다. 어빌리티가
	// 서버에서 자기 InventoryComponent 기준으로 이 ID의 아이템을 다시 찾아 표시 메시·장착 여부를 조회한다.
	// 이 아이템이 실제로 내 인벤토리 소속인지(예: 남의 시체를 루팅 중인 창이 아닌지)는 서버의
	// UA1Ability_DropItem::DropItemAuth가 다시 검증하므로 여기서는 별도로 확인하지 않는다.
	FGameplayEventData Payload;
	Payload.EventTag = A1GameplayTags::GameplayEvent_DropItem;
	Payload.Instigator = OwningPawn;
	Payload.EventMagnitude = static_cast<float>(DragOp->ItemInstance->ItemId);

	ASC->HandleGameplayEvent(A1GameplayTags::GameplayEvent_DropItem, &Payload);

	return true;
}
