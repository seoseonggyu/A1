// Copyright Epic Games, Inc. All Rights Reserved.

#include "A1Ability_Consume.h"

#include "Equipment/EquipmentComponent.h"
#include "Equipment/EquipmentInstance.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemInstance.h"
#include "GameFramework/Pawn.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_Consume)

DEFINE_LOG_CATEGORY(A1Ability_ConsumeLog);

UA1Ability_Consume::UA1Ability_Consume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

UEquipmentInstance* UA1Ability_Consume::GetConsumableEquipmentInstance() const
{
	const APawn* Pawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (Pawn == nullptr)
	{
		return nullptr;
	}

	UEquipmentComponent* EquipmentComp = UEquipmentComponent::FindEquipmentComponent(Pawn);
	if (EquipmentComp == nullptr)
	{
		return nullptr;
	}

	// 소비형 아이템은 사용 시점에 손에 들려 있는(활성) 메인 장비다.
	return EquipmentComp->GetActiveMainEquippedItem();
}

// 소비형 장비의 원본 아이템을 1개 줄인다. 스택이 남으면 수량만 감소시키고,
// 0이 되면 장비 슬롯을 해제한 뒤 인벤토리에서 아이템을 제거한다. (전부 서버 권한에서만 처리)
void UA1Ability_Consume::ConsumeSourceItemAuth()
{
	if (HasAuthority(&CurrentActivationInfo) == false)
	{
		return;
	}

	UEquipmentInstance* Consumable = GetConsumableEquipmentInstance();
	if (Consumable == nullptr)
	{
		return;
	}

	UItemInstance* SourceItem = Consumable->GetSourceItemInstance();
	if (SourceItem == nullptr)
	{
		return;
	}

	const APawn* Pawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	UInventoryComponent* Inventory = UInventoryComponent::FindInventoryComponent(Pawn);
	if (Inventory == nullptr)
	{
		UE_LOG(A1Ability_ConsumeLog, Warning, TEXT("ConsumeSourceItemAuth: InventoryComponent를 찾을 수 없습니다."));
		return;
	}

	const FInventoryEntry* Entry = Inventory->FindEntry(SourceItem);
	const int32 CurrentStack = Entry ? Entry->StackCount : 0;

	if (CurrentStack > 1)
	{
		// 아직 남았으면 수량만 1 감소시킨다.
		Inventory->ModifyStackCountAuth(SourceItem, CurrentStack - 1);
	}
	else
	{
		// 마지막 1개를 소비하면 장비 슬롯을 먼저 해제한 뒤 아이템을 제거한다.
		// (장비가 이미 제거된 아이템을 참조한 채 남지 않도록 해제 → 제거 순서를 지킨다)
		const FGameplayTag SlotTag = Consumable->GetEquipmentSlotTag();
		if (SlotTag.IsValid())
		{
			if (UEquipmentComponent* EquipmentComp = UEquipmentComponent::FindEquipmentComponent(Pawn))
			{
				EquipmentComp->UnequipItemAuth(SlotTag);
			}
		}

		Inventory->RemoveItemAuth(SourceItem);
	}
}
