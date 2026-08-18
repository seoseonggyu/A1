// Copyright Epic Games, Inc. All Rights Reserved.

#include "Equipment/EquipmentComponent.h"
#include "Equipment/EquipmentInstance.h"
#include "Equipment/EquipmentDefinition.h"
#include "Equipment/Fragment/EquipmentFragment_Ability.h"
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
	// 서버가 bActive를 토글하면(메인 장비 전환) 클라이언트도 액터를 스폰/제거해 맞춥니다.
	if (InArraySerializer.Owner)
	{
		InArraySerializer.Owner->ReconcileReplicatedEntryActors(const_cast<FEquipmentEntry*>(this));
	}
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

		// 삭제되기 전에, 이 엔트리가 현재 손에 든 메인 장비였는지 기록해둡니다(삭제 후엔 알 수 없음).
		const bool bWasActiveMainItem = Instance->IsEquipmentActive() && Instance->GetQuickBarSlotTag().IsValid();

		// 활성 상태였을 때만 실제 해제됩니다(멱등). 비활성(비메인) 엔트리면 무시됩니다.
		Instance->DeactivateEquipment();

		// 대체 없이(=SetMainEquippedAuth를 거치지 않고) 손에 든 메인 장비가 통째로 사라진 경우, UI에 알립니다.
		if (bWasActiveMainItem && InArraySerializer.Owner)
		{
			InArraySerializer.Owner->OnMainEquippedItemChanged.Broadcast(nullptr);
		}
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
	if (!GetOwner()->HasAuthority())
	{
		co_return;
	}
	if (!Item)
	{
		co_return;
	}

	const FItemFragment_Equipment* Fragment = Item->FindFragment<FItemFragment_Equipment>();
	if (!Fragment || !Fragment->EquipmentDefinition)
	{
		co_return;
	}

	const FGameplayTag SlotTag = Fragment->EquipmentDefinition->SlotTag;

	// 같은 Equipment 슬롯에 이미 장비가 있으면 먼저 해제합니다(방어구 교체 시나리오).
	// 메인 장비류(검/물약)는 각자 고유한 Equipment 슬롯을 쓰므로 서로를 밀어내지 않고,
	// 스폰 여부는 EquipItemAuthInternal의 메인 장비 전환 로직이 담당합니다.
	UnequipItemAuth(SlotTag);

	co_await EquipItemAuthCoroutine(Item);
}

void UEquipmentComponent::UnequipItemAuth(FGameplayTag SlotTag)
{
	// 해당 Equipment 슬롯에 장비가 없으면 아무것도 하지 않습니다.
	UEquipmentInstance* Instance = GetEquipmentInSlot(SlotTag);
	if (!Instance)
	{
		return;
	}

	FEquipmentEntry* Entry = FindEntryByInstance(Instance);
	if (!Entry)
	{
		UE_LOG(EquipmentComponentLog, Warning, TEXT("UnequipItemAuth: 장착 목록에서 찾을 수 없습니다"));
		return;
	}

	// 메인 장비였다면 참조를 정리하고, 대체 없이 사라짐을 UI에 알립니다.
	if (MainEquippedItem == Instance)
	{
		MainEquippedItem = nullptr;
		OnMainEquippedItemChanged.Broadcast(nullptr);
	}

	// 슬롯 맵(데이터)에서 제거합니다.
	RemoveFromSlotMap(Instance);

	// 액터/Fragment 해제 (활성 상태였을 때만 실제 동작, 멱등).
	Instance->DeactivateEquipment();

	// 목록에서 제거합니다.
	const int32 EntryIndex = static_cast<int32>(Entry - EquipmentList.Entries.GetData());
	EquipmentList.Entries.RemoveAt(EntryIndex);
	EquipmentList.MarkArrayDirty();
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

	// 슬롯 맵(데이터)에는 항상 등록합니다 (UI/조회용).
	AddToSlotMap(NewInstance);

	// 활성(메인 또는 방어구)일 때만 액터를 스폰합니다.
	// 비활성(비메인 메인 장비)이면 데이터만 유지하고, 이후 bActive가 true로 바뀌면
	// PostReplicatedChange(ReconcileReplicatedEntryActors)에서 스폰됩니다.
	if (CurrentEntry->bActive)
	{
		NewInstance->ActivateEquipment();

		if (NewInstance->GetQuickBarSlotTag().IsValid())
		{
			OnMainEquippedItemChanged.Broadcast(NewInstance);
		}
	}

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


bool UEquipmentComponent::CanSetActiveSlot(FGameplayTag QuickBarSlotTag) const
{
	if (!QuickBarSlotTag.IsValid())
	{
		return false;
	}

	// 이미 활성화된 메인 장비의 슬롯이면 재선택을 무시합니다.
	// (MainEquippedItem은 서버 전용이라 클라이언트에서는 항상 통과 후 서버가 최종 검증합니다.)
	if (MainEquippedItem && MainEquippedItem->GetQuickBarSlotTag() == QuickBarSlotTag)
	{
		return false;
	}

	return true;
}

void UEquipmentComponent::SetActiveSlotServer_Implementation(FGameplayTag QuickBarSlotTag)
{
	if (!CanSetActiveSlot(QuickBarSlotTag))
	{
		return;
	}

	// 이미 장착(데이터 등록)된 장비 중 해당 QuickBar 슬롯을 쓰는 것을 메인으로 전환합니다.
	UEquipmentInstance* Target = FindEquipmentByQuickBarSlot(QuickBarSlotTag);
	if (!Target)
	{
		return;
	}

	SetMainEquippedAuth(Target);
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

	// Entry 설정 (리플리케이션 데이터)
	FEquipmentEntry& NewEntry = EquipmentList.Entries.AddDefaulted_GetRef();
	NewEntry.Definition = Definition;
	NewEntry.SourceItemId = Item->ItemId;

	// Instance 생성
	UEquipmentInstance* NewInstance = CreateEquipmentInstance(Definition, Item->ItemId);
	NewEntry.Instance = NewInstance;

	// 슬롯 맵(데이터)에 등록합니다.
	AddToSlotMap(NewInstance);

	if (NewInstance->GetQuickBarSlotTag().IsValid())
	{
		// 손에 드는 메인 장비: 데이터만 먼저 확정하고, 실제 스폰은 메인 전환 로직이 처리합니다.
		// (한 번에 하나만 스폰되며, 이전 메인은 데이터 유지한 채 액터만 해제됩니다.)
		NewEntry.bActive = false;
		EquipmentList.MarkEntryDirty(NewEntry);

		SetMainEquippedAuth(NewInstance);
	}
	else
	{
		// 방어구류: 즉시 활성화하여 계속 착용합니다 (서버도 히트 판정 등을 위해 스폰).
		NewEntry.bActive = true;
		EquipmentList.MarkEntryDirty(NewEntry);

		NewInstance->ActivateEquipment();
	}

	return NewInstance;
}

void UEquipmentComponent::SetMainEquippedAuth(UEquipmentInstance* NewMain)
{
	if (MainEquippedItem == NewMain)
	{
		return;
	}

	// 이전 메인 장비의 액터만 해제합니다. 데이터(엔트리/슬롯 맵)는 그대로 유지합니다.
	if (MainEquippedItem)
	{
		if (FEquipmentEntry* OldEntry = FindEntryByInstance(MainEquippedItem))
		{
			SetEntryActiveAuth(*OldEntry, false);
		}
	}

	MainEquippedItem = NewMain;

	// 새 메인 장비의 액터를 스폰합니다.
	if (NewMain)
	{
		if (FEquipmentEntry* NewEntry = FindEntryByInstance(NewMain))
		{
			SetEntryActiveAuth(*NewEntry, true);
		}
	}
}

void UEquipmentComponent::SetEntryActiveAuth(FEquipmentEntry& Entry, bool bNewActive)
{
	if (Entry.bActive == bNewActive)
	{
		return;
	}

	Entry.bActive = bNewActive;

	// 서버에서 액터를 직접 스폰/제거합니다 (클라이언트는 PostReplicatedChange에서 반영).
	if (Entry.Instance)
	{
		if (bNewActive)
		{
			Entry.Instance->ActivateEquipment();

			if (Entry.Instance->GetQuickBarSlotTag().IsValid())
			{
				OnMainEquippedItemChanged.Broadcast(Entry.Instance);
			}
		}
		else
		{
			Entry.Instance->DeactivateEquipment();
		}
	}

	EquipmentList.MarkEntryDirty(Entry);
}

void UEquipmentComponent::ReconcileReplicatedEntryActors(FEquipmentEntry* Entry)
{
	// Instance가 아직 없으면(초기화 코루틴 진행 중) 무시합니다.
	// 코루틴 완료 시 현재 bActive 값을 읽어 스폰 여부를 결정하므로 여기서 처리할 필요가 없습니다.
	if (!Entry || !Entry->Instance)
	{
		return;
	}

	// 두 메서드 모두 멱등이라 중복 호출은 안전하게 무시됩니다.
	if (Entry->bActive)
	{
		Entry->Instance->ActivateEquipment();

		if (Entry->Instance->GetQuickBarSlotTag().IsValid())
		{
			OnMainEquippedItemChanged.Broadcast(Entry->Instance);
		}
	}
	else
	{
		Entry->Instance->DeactivateEquipment();
	}
}

FEquipmentEntry* UEquipmentComponent::FindEntryByInstance(const UEquipmentInstance* Instance)
{
	if (!Instance)
	{
		return nullptr;
	}

	for (FEquipmentEntry& Entry : EquipmentList.Entries)
	{
		if (Entry.Instance == Instance)
		{
			return &Entry;
		}
	}

	return nullptr;
}

UEquipmentInstance* UEquipmentComponent::FindEquipmentByQuickBarSlot(FGameplayTag QuickBarSlotTag) const
{
	if (!QuickBarSlotTag.IsValid())
	{
		return nullptr;
	}

	for (const FEquipmentEntry& Entry : EquipmentList.Entries)
	{
		if (Entry.Instance && Entry.Instance->GetQuickBarSlotTag() == QuickBarSlotTag)
		{
			return Entry.Instance;
		}
	}

	return nullptr;
}

UEquipmentInstance* UEquipmentComponent::GetEquipmentInSlot(FGameplayTag SlotTag) const
{
	if (const TObjectPtr<UEquipmentInstance>* Found = EquipmentSlots.Find(SlotTag))
	{
		return *Found;
	}

	return nullptr;
}

UEquipmentInstance* UEquipmentComponent::GetActiveMainEquippedItem() const
{
	// MainEquippedItem은 서버 전용이라, 서버·클라이언트 모두 동작하도록 EquipmentSlots(데이터)에서
	// 직접 "QuickBarSlotTag가 있고 현재 활성화된" 장비를 찾습니다.
	for (const auto& Pair : EquipmentSlots)
	{
		UEquipmentInstance* Instance = Pair.Value;
		if (Instance && Instance->IsEquipmentActive() && Instance->GetQuickBarSlotTag().IsValid())
		{
			return Instance;
		}
	}

	return nullptr;
}

UEquipmentInstance* UEquipmentComponent::FindEquippedItemWithAbility(FGameplayTag InputTag) const
{
	if (!InputTag.IsValid())
	{
		return nullptr;
	}

	for (const auto& Pair : EquipmentSlots)
	{
		UEquipmentInstance* Instance = Pair.Value;
		if (!Instance)
		{
			continue;
		}

		const UEquipmentDefinition* Definition = Instance->GetDefinition();
		const FEquipmentFragment_Ability* Fragment = Definition ? Definition->FindFragment<FEquipmentFragment_Ability>() : nullptr;
		if (!Fragment)
		{
			continue;
		}

		for (const FCommonAbilityEntry& Entry : Fragment->Abilities)
		{
			if (Entry.InputTag == InputTag)
			{
				return Instance;
			}
		}
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
		if (EquipmentSlots[SlotTag] == Instance)
		{
			EquipmentSlots[SlotTag] = nullptr;

			OnEquipmentSlotChanged.Broadcast(SlotTag, nullptr);
		}
	}
}
