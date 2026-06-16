// Copyright Epic Games, Inc. All Rights Reserved.

#include "Inventory/ItemInstance.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemDefinition.h"
#include "Inventory/Fragment/ItemFragment_Equipment.h"
#include "Equipment/EquipmentDefinition.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(ItemInstance)

DEFINE_LOG_CATEGORY(ItemInstanceLog);

void UItemInstance::InitializeFragmentsFromDefinition()
{
	if (!Definition)
	{
		UE_LOG(ItemInstanceLog, Warning, TEXT("Definition이 nullptr입니다"));
		return;
	}

	Fragments.Reset();

	// Definition의 Fragment들을 복사
	for (const TInstancedStruct<FItemFragment>& SourceFragment : Definition->Fragments)
	{
		TInstancedStruct<FItemFragment>& NewFragment = Fragments.Add_GetRef(SourceFragment);
		if (FItemFragment* Frag = NewFragment.GetMutablePtr<FItemFragment>())
		{
			Frag->OnCreated(this);
		}
	}
}

FItemFragment* UItemInstance::FindFragmentByIndex(int32 Index)
{
	if (Fragments.IsValidIndex(Index))
	{
		return Fragments[Index].GetMutablePtr<FItemFragment>();
	}

	return nullptr;
}

int32 UItemInstance::GetFragmentIndex(const UScriptStruct* InFragmentType) const
{
	if (!InFragmentType)
	{
		return INDEX_NONE;
	}

	for (int32 i = 0; i < Fragments.Num(); ++i)
	{
		const UScriptStruct* EntryStruct = Fragments[i].GetScriptStruct();
		if (EntryStruct && EntryStruct->IsChildOf(InFragmentType))
		{
			return i;
		}
	}

	return INDEX_NONE;
}

void UItemInstance::ResetFragmentToDefault(int32 FragmentIdx)
{
	if (!Definition || !Definition->Fragments.IsValidIndex(FragmentIdx))
	{
		return;
	}

	// Definition(CDO)에서 기본 Fragment 가져오기
	const TInstancedStruct<FItemFragment>& DefaultEntry = Definition->Fragments[FragmentIdx];
	const FItemFragment* DefaultFragment = DefaultEntry.GetPtr<FItemFragment>();
	const UScriptStruct* FragmentStruct = DefaultEntry.GetScriptStruct();

	if (!DefaultFragment || !FragmentStruct)
	{
		return;
	}

	FItemFragment* Fragment = FindFragmentByIndex(FragmentIdx);
	if (!Fragment)
	{
		return;
	}

	// Definition의 기본값으로 복원
	FragmentStruct->CopyScriptStruct(Fragment, DefaultFragment);
	Fragment->OnChanged(this);
}

bool UItemInstance::HasAuthority() const
{
	const UInventoryComponent* Inventory = Cast<UInventoryComponent>(GetOuter());
	return Inventory && Inventory->GetOwner() && Inventory->GetOwner()->HasAuthority();
}

FGameplayTag UItemInstance::GetEquipmentSlotTag() const
{
	const FItemFragment_Equipment* Fragment = FindFragment<FItemFragment_Equipment>();
	if (!Fragment || !Fragment->EquipmentDefinition)
	{
		return FGameplayTag();
	}

	return Fragment->EquipmentDefinition->SlotTag;
}

FGameplayTag UItemInstance::GetQuickBarSlotTag() const
{
	const FItemFragment_Equipment* Fragment = FindFragment<FItemFragment_Equipment>();
	if (!Fragment || !Fragment->EquipmentDefinition)
	{
		return FGameplayTag();
	}

	return Fragment->EquipmentDefinition->QuickBarSlotTag;
}

int32 UItemInstance::FindTagStatIndex(FGameplayTag InStatTag) const
{
	for (int32 i = 0; i < Fragments.Num(); ++i)
	{
		if (const FFragment_TagStat* Found = Fragments[i].GetPtr<FFragment_TagStat>())
		{
			if (Found->StatTag.MatchesTagExact(InStatTag))
			{
				return i;
			}
		}
	}

	return INDEX_NONE;
}

FItemNetStateList* UItemInstance::GetOwnerNetStates() const
{
	UInventoryComponent* Inventory = Cast<UInventoryComponent>(GetOuter());
	return Inventory ? &Inventory->ItemNetStates : nullptr;
}

bool UItemInstance::RemoveNetStateByIndex(int32 FragmentIdx)
{
	FItemNetStateList* NetStates = GetOwnerNetStates();
	if (FragmentIdx == INDEX_NONE || !NetStates)
	{
		return false;
	}

	int32 EntryIndex = NetStates->FindIndex(ItemId, static_cast<uint8>(FragmentIdx));
	if (EntryIndex == INDEX_NONE)
	{
		return false;
	}

	ResetFragmentToDefault(FragmentIdx);

	NetStates->Entries.RemoveAtSwap(EntryIndex);
	NetStates->MarkArrayDirty();

	return true;
}

TItemFragmentNetStateModifier<FNetState_TagStat> UItemInstance::ModifyTagStatAuth(FGameplayTag InStatTag)
{
	if (!HasAuthority())
	{
		return {};
	}

	return ModifyNetStateByIndexAuth<FNetState_TagStat, FFragment_TagStat>(FindTagStatIndex(InStatTag));
}

bool UItemInstance::RemoveTagStatAuth(FGameplayTag InStatTag)
{
	if (!HasAuthority())
	{
		return false;
	}

	return RemoveNetStateByIndex(FindTagStatIndex(InStatTag));
}

bool UItemInstance::GetTagStatValue(FGameplayTag InStatTag, float& OutValue) const
{
	int32 Index = FindTagStatIndex(InStatTag);
	if (Index == INDEX_NONE)
	{
		return false;
	}

	if (const FFragment_TagStat* Found = Fragments[Index].GetPtr<FFragment_TagStat>())
	{
		OutValue = Found->Value;
		return true;
	}

	return false;
}

bool UItemInstance::SetTagStatValueLocal(FGameplayTag InStatTag, float NewValue)
{
	int32 Index = FindTagStatIndex(InStatTag);
	if (Index == INDEX_NONE)
	{
		return false;
	}

	if (FFragment_TagStat* Found = Fragments[Index].GetMutablePtr<FFragment_TagStat>())
	{
		Found->Value = NewValue;
		Found->OnChanged(this);
		return true;
	}

	return false;
}
