// Copyright Epic Games, Inc. All Rights Reserved.

#include "Equipment/EquipmentComponent.h"
#include "Equipment/EquipmentInstance.h"
#include "Equipment/EquipmentDefinition.h"
#include "Inventory/ItemInstance.h"
#include "Inventory/Fragment/ItemFragment_Equipment.h"
#include "Experience/ExperienceManagerComponent.h"
#include "Coroutine/CommonAssetAwaiters.h"
#include "Inventory/InventoryComponent.h"
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
	// 여기서 진행 중인 init 코루틴을 SlotTag로 취소하지 않습니다.
	// FastArray(Iris)는 같은 델타의 add/remove 콜백 순서를 보장하지 않으므로,
	// 같은 슬롯으로 무기를 교체할 때 remove(old)가 방금 등록된 new의 코루틴을
	// 취소해버릴 수 있습니다. orphan 방지는 코루틴의 re-find 가드
	// (InitializeReplicatedEquipmentCoroutine에서 SourceItemId로 재조회)가 담당하고,
	// 같은 슬롯의 옛 태스크 취소는 StartReplicatedEquipmentInit(add 시점)이 담당합니다.
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

TCoroTask<void> UEquipmentComponent::HandleActiveEquipChangedAuth(UItemInstance* Item)
{
	if (!GetOwner()->HasAuthority()) co_return;
	
	const FItemFragment_Equipment* Fragment = Item->FindFragment<FItemFragment_Equipment>();
	const UEquipmentDefinition* Definition = Fragment->EquipmentDefinition;
	const FGameplayTag& SlotTag = Definition->SlotTag;
	
	UnequipItemAuth(SlotTag);
	
	// TODO: 현재 Main아이템 처리 필요
	TObjectPtr<UEquipmentInstance> EquippedItem = nullptr;
	EquippedItem = co_await EquipItemAuthCoroutine(Item);
	
}

void UEquipmentComponent::UnequipItemAuth(FGameplayTag SlotTag)
{
	// 같은 슬롯에 장비가 있으면 해제
	if (UEquipmentInstance* Instance = GetEquipmentInSlot(SlotTag))
	{
		if (!Instance)
		{
			return;
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
			return;
		}

		// 슬롯 맵에서 제거합니다
		RemoveFromSlotMap(Instance);

		// 서버에서 Fragment 콜백을 호출합니다 (클라이언트는 PreReplicatedRemove에서 호출됨)
		Instance->OnUnequipped();
	
		// 목록에서 제거합니다
		EquipmentList.Entries.RemoveAtSwap(EntryIndex);
		EquipmentList.MarkArrayDirty();
	}
}

void UEquipmentComponent::StartReplicatedEquipmentInit(FEquipmentEntry* Entry)
{
	if (!Entry || !Entry->Definition)
	{
		return;
	}

	const FGameplayTag SlotTag = Entry->Definition->SlotTag;

	// 같은 슬롯의 기존(옛) 태스크 취소.
	// 취소는 반드시 이 add 경로에서만 합니다. add 시점의 "기존 태스크"는 항상 교체 대상인
	// 옛 장비의 것이므로 안전합니다. 반대로 PreReplicatedRemove(제거)에서 SlotTag로 취소하면,
	// FastArray(Iris)가 add/remove 콜백 순서를 보장하지 않아 방금 등록된 새 장비의 태스크를
	// 잘못 죽일 수 있습니다. (그래서 제거 경로에는 취소를 두지 않습니다)
	if (TCoroTask<void>* ExistingTask = PendingInitTasks.Find(SlotTag))
	{
		ExistingTask->Cancel();
		PendingInitTasks.Remove(SlotTag);
	}

	// 새 태스크 시작 및 저장
	TCoroTask<void> NewTask = InitializeReplicatedEquipmentCoroutine(Entry);
	PendingInitTasks.Add(SlotTag, MoveTemp(NewTask));
}

/**
 * 복제된 Entry에 대해 클라이언트에서 로컬 EquipmentInstance를 생성/초기화합니다.
 *
 * [처음 보는 사람을 위한 배경]
 * 이 코루틴은 파라미터로 받은 FEquipmentEntry* Entry 를 co_await 너머까지 그대로
 * 사용하면 안 됩니다. Entry는 EquipmentList.Entries(TArray)의 원소를 가리키는
 * 생포인터인데, 대기(co_await) 동안 서버가 다른 장비를 장착/해제하면 클라이언트
 * FastArray가 이 배열에 add/remove를 적용하면서 재할당·이동이 일어나고, 그 순간
 * Entry는 dangling(무효) 포인터가 됩니다.
 *
 * 이 함정이 특히 헷갈렸던 이유:
 *  - 아래 SpawnEquipmentActors()는 "로컬 변수 NewInstance"로 동작하므로 항상 성공 →
 *    에디터/월드에는 무기 액터가 실제로 보입니다.
 *  - 반면 "CurrentEntry->Instance = NewInstance" 기록은 포인터를 거치는데, 예전처럼
 *    dangling Entry에 쓰면 이 값이 엉뚱한 메모리로 새어나가 살아있는 엔트리에는
 *    Instance가 null로 남습니다.
 *  - 결과: 액터는 떠 있는데 PreReplicatedRemove에서 Instance가 null → 그 액터를
 *    파괴하지 못하는 orphan 발생. (배열 재할당 여부에 따라 "가끔"만 재현되던 UB)
 *
 * 그래서: co_await 전에 안정적인 값(Definition/SlotTag/SourceItemId)만 복사해 두고,
 * 대기가 끝나면 SourceItemId로 살아있는 엔트리를 "다시 찾아" 거기에만 기록합니다.
 * 이러면 스폰과 기록이 항상 같은(살아있는) 대상을 가리키게 됩니다.
 */
TCoroTask<void> UEquipmentComponent::InitializeReplicatedEquipmentCoroutine(FEquipmentEntry* Entry)
{
	if (!Entry || !Entry->Definition)
	{
		co_return;
	}

	// co_await 이후 Entry 포인터는 무효화될 수 있으므로(위 주석 참고), 여기서 안정적인
	// 식별자를 값으로 복사해 둡니다. 이 지점 이후로는 Entry 포인터를 절대 사용하지 않습니다.
	const TObjectPtr<const UEquipmentDefinition> Definition = Entry->Definition;
	const FGameplayTag SlotTag = Definition->SlotTag;
	const int32 SourceItemId = Entry->SourceItemId;

	// Experience 로딩 대기 (PrimaryGameLayout 준비)
	co_await UExperienceManagerComponent::WaitForExperienceLoadedStaticCoroutine(this);

	// 에셋 로딩 대기
	co_await Coro::Async::LoadCommonDataAsset<UEquipmentDefinition>(this, Definition);

	// [핵심 가드] 대기 중 이 장비가 해제됐을 수 있으므로, 원본 Entry 포인터 대신
	// SourceItemId로 "지금 살아있는" 엔트리를 다시 찾습니다.
	// 못 찾으면(=대기 중 UnequipItemAuth로 제거됨) 인스턴스를 만들지도, 액터를 스폰하지도
	// 않고 종료합니다. 이 한 줄이 orphan 액터/댕글링 쓰기를 원천 차단합니다.
	FEquipmentEntry* CurrentEntry = FindEntryBySourceItemId(SourceItemId);
	if (!CurrentEntry || CurrentEntry->Definition != Definition)
	{
		PendingInitTasks.Remove(SlotTag);
		co_return;
	}

	// Instance 생성 및 초기화 (기록은 반드시 "다시 찾은" CurrentEntry에만 합니다)
	UEquipmentInstance* NewInstance = CreateEquipmentInstance(Definition, SourceItemId);
	CurrentEntry->Instance = NewInstance;

	// 슬롯 맵에 등록 및 초기화
	AddToSlotMap(NewInstance);
	NewInstance->SpawnEquipmentActors();
	NewInstance->OnEquipped();
	
	// 완료된 태스크 정리
	PendingInitTasks.Remove(SlotTag);
}

FEquipmentEntry* UEquipmentComponent::FindEntryBySourceItemId(int32 SourceItemId)
{
	if (SourceItemId == INDEX_NONE)
	{
		return nullptr;
	}

	for (FEquipmentEntry& Entry : EquipmentList.Entries)
	{
		if (Entry.SourceItemId == SourceItemId)
		{
			return &Entry;
		}
	}

	return nullptr;
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

	// 히트 판정을 위해 서버에서도 장비 액터를 스폰
	NewInstance->SpawnEquipmentActors();

	// 서버에서 Fragment 콜백을 호출합니다 (클라이언트는 PostReplicatedAdd에서 호출됨)
	NewInstance->OnEquipped();
	
	return NewInstance;
}


void UEquipmentComponent::UnequipAllAuth()
{
	while (EquipmentList.Entries.Num() > 0)
	{
		break;
		// UnequipItemAuth(EquipmentList.Entries[0].Instance);
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

		// UI가 슬롯 아이콘을 갱신하도록 알림 (서버/클라이언트 모두 이 경로를 지남)
		OnEquipmentSlotChanged.Broadcast(SlotTag, Instance);
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

			// UI가 슬롯 아이콘을 비우도록 알림 (서버/클라이언트 모두 이 경로를 지남)
			OnEquipmentSlotChanged.Broadcast(SlotTag, nullptr);
		}
	}
}
