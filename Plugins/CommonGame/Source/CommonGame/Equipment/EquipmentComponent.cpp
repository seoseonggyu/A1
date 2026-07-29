// Copyright Epic Games, Inc. All Rights Reserved.

#include "Equipment/EquipmentComponent.h"
#include "Equipment/EquipmentInstance.h"
#include "Equipment/EquipmentDefinition.h"
#include "Inventory/ItemInstance.h"
#include "Inventory/Fragment/ItemFragment_Equipment.h"
#include "Experience/ExperienceManagerComponent.h"
#include "Coroutine/CommonAssetAwaiters.h"
#include "Net/UnrealNetwork.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(EquipmentComponent)

DEFINE_LOG_CATEGORY(EquipmentComponentLog);

//-----------------------------------------------------------------------------
// FEquipmentEntry
//-----------------------------------------------------------------------------

void FEquipmentEntry::PostReplicatedAdd(const FEquipmentList& InArraySerializer)
{
	if (!Definition)
	{
		return;
	}

	if (InArraySerializer.Owner)
	{
		InArraySerializer.Owner->StartReplicatedEquipmentInit(const_cast<FEquipmentEntry*>(this));
	}
}

void FEquipmentEntry::PostReplicatedChange(const FEquipmentList& InArraySerializer)
{
}

void FEquipmentEntry::PreReplicatedRemove(const FEquipmentList& InArraySerializer)
{
	if (Instance)
	{
		// 슬롯 맵에서 제거합니다
		if (InArraySerializer.Owner)
		{
			InArraySerializer.Owner->RemoveFromSlotMap(Instance);
		}

		Instance->OnUnequipped();
		Instance->DestroyEquipmentActors();
	}
}

//-----------------------------------------------------------------------------
// UEquipmentComponent
//-----------------------------------------------------------------------------

UEquipmentComponent::UEquipmentComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	EquipmentList.Owner = this;
}

UEquipmentComponent* UEquipmentComponent::FindEquipmentComponent(const APawn* Pawn)
{
	return Pawn ? Pawn->FindComponentByClass<UEquipmentComponent>() : nullptr;
}

void UEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, EquipmentList, Params);
}

void UEquipmentComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 진행 중인 초기화 태스크 취소
	for (auto& Pair : PendingInitTasks)
	{
		Pair.Value.Cancel();
	}
	PendingInitTasks.Empty();

	// 모든 장비 액터 파괴 (비정상 종료 및 클라이언트 시뮬레이티드 폰 대비)
	for (FEquipmentEntry& Entry : EquipmentList.Entries)
	{
		if (Entry.Instance)
		{
			Entry.Instance->DestroyEquipmentActors();
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UEquipmentComponent::StartReplicatedEquipmentInit(FEquipmentEntry* Entry)
{
	if (!Entry || !Entry->Definition)
	{
		return;
	}

	const FGameplayTag SlotTag = Entry->Definition->SlotTag;

	// 같은 슬롯의 기존 태스크 취소
	if (TCoroTask<void>* ExistingTask = PendingInitTasks.Find(SlotTag))
	{
		ExistingTask->Cancel();
		PendingInitTasks.Remove(SlotTag);
	}

	// 새 태스크 시작 및 저장
	TCoroTask<void> NewTask = InitializeReplicatedEquipmentCoroutine(Entry);
	PendingInitTasks.Add(SlotTag, MoveTemp(NewTask));
}

TCoroTask<void> UEquipmentComponent::InitializeReplicatedEquipmentCoroutine(FEquipmentEntry* Entry)
{
	if (!Entry || !Entry->Definition)
	{
		co_return;
	}

	// Experience 로딩 대기 (PrimaryGameLayout 준비)
	co_await UExperienceManagerComponent::WaitForExperienceLoadedStaticCoroutine(this);

	// 에셋 로딩 대기
	co_await Coro::Async::LoadCommonDataAsset<UEquipmentDefinition>(this, Entry->Definition);

	// Instance 생성 및 초기화
	UEquipmentInstance* NewInstance = CreateEquipmentInstance(Entry->Definition, Entry->SourceItemId);
	Entry->Instance = NewInstance;

	// 슬롯 맵에 등록 및 초기화
	AddToSlotMap(NewInstance);
	NewInstance->SpawnEquipmentActors();
	NewInstance->OnEquipped();

	// 완료된 태스크 정리
	PendingInitTasks.Remove(Entry->Definition->SlotTag);
}

TCoroTask<UEquipmentInstance*> UEquipmentComponent::EquipItemAuthCoroutine(UItemInstance* Item)
{
	if (!Item)
	{
		UE_LOG(EquipmentComponentLog, Warning, TEXT("EquipItemAuthCoroutine: Item이 nullptr입니다"));
		co_return nullptr;
	}

	const FItemFragment_Equipment* Fragment = Item->FindFragment<FItemFragment_Equipment>();
	if (!Fragment)
	{
		UE_LOG(EquipmentComponentLog, Warning, TEXT("EquipItemAuthCoroutine: Item에 FItemFragment_Equipment가 없습니다"));
		co_return nullptr;
	}

	const UEquipmentDefinition* Definition = Fragment->EquipmentDefinition;
	if (!Definition)
	{
		UE_LOG(EquipmentComponentLog, Warning, TEXT("EquipItemAuthCoroutine: EquipmentDefinition이 설정되지 않았습니다"));
		co_return nullptr;
	}

	const FGameplayTag& SlotTag = Definition->SlotTag;
	if (!SlotTag.IsValid())
	{
		UE_LOG(EquipmentComponentLog, Warning, TEXT("EquipItemAuthCoroutine: SlotTag가 유효하지 않습니다"));
		co_return nullptr;
	}

	if (!EquipmentSlots.Contains(SlotTag))
	{
		UE_LOG(EquipmentComponentLog, Warning, TEXT("EquipItemAuthCoroutine: 등록되지 않은 슬롯입니다 (%s)"), *SlotTag.ToString());
		co_return nullptr;
	}

	co_await Coro::Async::LoadCommonDataAsset<UEquipmentDefinition>(this, Definition);

	UEquipmentInstance* Instance = EquipItemAuthInternal(Item);
	co_return Instance;
}

UEquipmentInstance* UEquipmentComponent::CreateEquipmentInstance(const UEquipmentDefinition* Definition, int32 SourceItemId) const
{
	// InstanceClass 결정 (설정된 클래스가 없으면 기본 UEquipmentInstance 사용)
	TSubclassOf<UEquipmentInstance> ClassToUse = Definition->InstanceClass ? Definition->InstanceClass.Get() : UEquipmentInstance::StaticClass();

	// Instance 생성 (Outer를 Actor로 설정하여 서버/클라이언트 동일하게 유지)
	UEquipmentInstance* NewInstance = NewObject<UEquipmentInstance>(GetOwner(), ClassToUse);
	NewInstance->Definition = Definition;
	NewInstance->SourceItemId = SourceItemId;

	return NewInstance;
}

UEquipmentInstance* UEquipmentComponent::EquipItemAuthInternal(UItemInstance* Item)
{
	const FItemFragment_Equipment* Fragment = Item->FindFragment<FItemFragment_Equipment>();
	const UEquipmentDefinition* Definition = Fragment->EquipmentDefinition;
	const FGameplayTag& SlotTag = Definition->SlotTag;

	// 같은 슬롯에 장비가 있으면 해제합니다
	if (UEquipmentInstance* ExistingEquipment = GetEquipmentInSlot(SlotTag))
	{
		UnequipItemAuth(ExistingEquipment);
	}

	// Entry 설정 (리플리케이션 데이터)
	FEquipmentEntry& NewEntry = EquipmentList.Entries.AddDefaulted_GetRef();
	NewEntry.Definition = Definition;
	NewEntry.SourceItemId = Item->ItemId;

	// Instance 생성 및 초기화
	UEquipmentInstance* NewInstance = CreateEquipmentInstance(Definition, Item->ItemId);
	NewEntry.Instance = NewInstance;
	EquipmentList.MarkItemDirty(NewEntry);

	// 슬롯 맵에 등록합니다
	AddToSlotMap(NewInstance);

	// 스폰 액터는 복제되지 않으므로 서버에서도 직접 스폰합니다 (클라이언트는 PostReplicatedAdd 경로에서 스폰됨)
	NewInstance->SpawnEquipmentActors();

	// 서버에서 Fragment 콜백을 호출합니다 (클라이언트는 PostReplicatedAdd에서 호출됨)
	NewInstance->OnEquipped();

	return NewInstance;
}

bool UEquipmentComponent::UnequipItemAuth(UEquipmentInstance* Instance)
{
	if (!Instance)
	{
		return false;
	}

	int32 EntryIndex = INDEX_NONE;
	for (int32 i = 0; i < EquipmentList.Entries.Num(); ++i)
	{
		if (EquipmentList.Entries[i].Instance == Instance)
		{
			EntryIndex = i;
			break;
		}
	}

	if (EntryIndex == INDEX_NONE)
	{
		UE_LOG(EquipmentComponentLog, Warning, TEXT("UnequipItemAuth: 장착 목록에서 찾을 수 없습니다"));
		return false;
	}

	// 슬롯 맵에서 제거합니다
	RemoveFromSlotMap(Instance);

	// 서버에서 Fragment 콜백을 호출합니다 (클라이언트는 PreReplicatedRemove에서 호출됨)
	Instance->OnUnequipped();

	// 서버에서 스폰한 장비 액터를 파괴합니다 (클라이언트는 PreReplicatedRemove에서 파괴됨)
	Instance->DestroyEquipmentActors();

	// 목록에서 제거합니다
	EquipmentList.Entries.RemoveAtSwap(EntryIndex);
	EquipmentList.MarkArrayDirty();

	return true;
}

void UEquipmentComponent::UnequipAllAuth()
{
	while (EquipmentList.Entries.Num() > 0)
	{
		UnequipItemAuth(EquipmentList.Entries[0].Instance);
	}
}

UEquipmentInstance* UEquipmentComponent::GetEquipmentInSlot(FGameplayTag SlotTag) const
{
	if (const TObjectPtr<UEquipmentInstance>* Found = EquipmentSlots.Find(SlotTag))
	{
		return *Found;
	}

	return nullptr;
}

AActor* UEquipmentComponent::GetEquipmentInstance(FGameplayTag SlotTag) const
{
	// TODO: Temporarily returns only the first Actor
	if (const UEquipmentInstance* Instance = GetEquipmentInSlot(SlotTag))
	{
		const TArray<AActor*>& SpawnedActors = Instance->GetSpawnedActors();
		if (SpawnedActors.IsValidIndex(0))
		{
			return SpawnedActors[0];
		}
	}
	
	return nullptr;
}

void UEquipmentComponent::AddToSlotMap(UEquipmentInstance* Instance)
{
	if (!Instance || !Instance->Definition)
	{
		return;
	}

	const FGameplayTag& SlotTag = Instance->Definition->SlotTag;
	if (SlotTag.IsValid() && EquipmentSlots.Contains(SlotTag))
	{
		EquipmentSlots[SlotTag] = Instance;
	}
}

void UEquipmentComponent::RemoveFromSlotMap(UEquipmentInstance* Instance)
{
	if (!Instance || !Instance->Definition)
	{
		return;
	}

	const FGameplayTag& SlotTag = Instance->Definition->SlotTag;
	if (SlotTag.IsValid() && EquipmentSlots.Contains(SlotTag))
	{
		// 현재 슬롯에 있는 장비가 제거하려는 장비와 같을 때만 nullptr로 설정합니다
		if (EquipmentSlots[SlotTag] == Instance)
		{
			EquipmentSlots[SlotTag] = nullptr;
		}
	}
}
