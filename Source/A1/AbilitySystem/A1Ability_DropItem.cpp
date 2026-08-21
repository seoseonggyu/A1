// Copyright Epic Games, Inc. All Rights Reserved.

#include "A1Ability_DropItem.h"

#include "A1GameplayTags.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Equipment/EquipmentComponent.h"
#include "Equipment/EquipmentDefinition.h"
#include "Equipment/EquipmentInstance.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Interaction/Actors/A1Interactable_Pickup.h"
#include "Inventory/Fragment/ItemFragment_Equipment.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/ItemDefinition.h"
#include "Inventory/ItemInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Ability_DropItem)

DEFINE_LOG_CATEGORY(A1AbilityDropItemLog);

namespace
{
	/** 컴포넌트 소유자(실제 액터 또는 CDO)에서 표시 메시를 찾는다. StaticMesh 우선, 없으면 SkeletalMesh. */
	void FindDisplayMeshFromComponentOwner(const AActor* ComponentOwner, UStaticMesh*& OutStaticMesh, USkeletalMesh*& OutSkeletalMesh)
	{
		if (ComponentOwner == nullptr)
		{
			return;
		}

		if (const UStaticMeshComponent* MeshComp = ComponentOwner->FindComponentByClass<UStaticMeshComponent>())
		{
			OutStaticMesh = MeshComp->GetStaticMesh();
		}

		if (OutStaticMesh == nullptr)
		{
			if (const USkeletalMeshComponent* SkelComp = ComponentOwner->FindComponentByClass<USkeletalMeshComponent>())
			{
				OutSkeletalMesh = SkelComp->GetSkeletalMeshAsset();
			}
		}
	}

	/** 블루프린트 액터 클래스의 SCS 템플릿(상위 클래스까지 탐색)에서 표시 메시를 찾는다. */
	void FindDisplayMeshFromSCS(const UClass* ActorClass, UStaticMesh*& OutStaticMesh, USkeletalMesh*& OutSkeletalMesh)
	{
		for (const UClass* CurClass = ActorClass; CurClass != nullptr; CurClass = CurClass->GetSuperClass())
		{
			const UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(CurClass);
			if (BPClass == nullptr || BPClass->SimpleConstructionScript == nullptr)
			{
				continue;
			}

			for (const USCS_Node* Node : BPClass->SimpleConstructionScript->GetAllNodes())
			{
				if (Node == nullptr)
				{
					continue;
				}

				if (const UStaticMeshComponent* MeshTemplate = Cast<UStaticMeshComponent>(Node->ComponentTemplate))
				{
					if (UStaticMesh* FoundMesh = MeshTemplate->GetStaticMesh())
					{
						OutStaticMesh = FoundMesh;
						return;
					}
				}

				if (const USkeletalMeshComponent* SkelTemplate = Cast<USkeletalMeshComponent>(Node->ComponentTemplate))
				{
					if (USkeletalMesh* FoundMesh = SkelTemplate->GetSkeletalMeshAsset())
					{
						OutSkeletalMesh = FoundMesh;
						return;
					}
				}
			}
		}
	}

	/** 액터 클래스에서 표시 메시를 찾는다. 네이티브(CDO)를 먼저 보고, 없으면 BP SCS 템플릿을 본다. */
	void FindDisplayMeshFromActorClass(const UClass* ActorClass, UStaticMesh*& OutStaticMesh, USkeletalMesh*& OutSkeletalMesh)
	{
		if (ActorClass == nullptr)
		{
			return;
		}

		FindDisplayMeshFromComponentOwner(ActorClass->GetDefaultObject<AActor>(), OutStaticMesh, OutSkeletalMesh);
		if (OutStaticMesh == nullptr && OutSkeletalMesh == nullptr)
		{
			FindDisplayMeshFromSCS(ActorClass, OutStaticMesh, OutSkeletalMesh);
		}
	}

	/**
	 * 드롭할 아이템의 표시 메시와 배치 스케일을 찾는 단일 진입점. 장착/인벤토리 두 경로가 이 함수 하나를
	 * 공유한다 — 손에 든 장비도 결국 같은 EquipmentDefinition::ActorsToSpawn으로 스폰되므로, 장착 여부와
	 * 무관하게 항상 아이템의 정의를 기준으로 조회하면 충분하다.
	 * 스케일은 FEquipmentActorToSpawn::AttachTransform에서 그대로 가져와, 손에 들었을 때와 동일한
	 * 크기로 드롭 픽업에 반영되게 한다(장착 시 UEquipmentInstance::SpawnActor가 이 스케일을 적용한다).
	 */
	void FindDisplayMeshForItem(const UItemInstance* Item, UStaticMesh*& OutStaticMesh, USkeletalMesh*& OutSkeletalMesh, FVector& OutDisplayScale)
	{
		OutStaticMesh = nullptr;
		OutSkeletalMesh = nullptr;
		OutDisplayScale = FVector::OneVector;

		if (Item == nullptr)
		{
			return;
		}

		const FItemFragment_Equipment* EquipFragment = Item->FindFragment<FItemFragment_Equipment>();
		if (EquipFragment == nullptr || EquipFragment->EquipmentDefinition == nullptr)
		{
			return;
		}

		for (const FEquipmentActorToSpawn& ActorToSpawn : EquipFragment->EquipmentDefinition->ActorsToSpawn)
		{
			FindDisplayMeshFromActorClass(ActorToSpawn.ActorClass, OutStaticMesh, OutSkeletalMesh);
			if (OutStaticMesh != nullptr || OutSkeletalMesh != nullptr)
			{
				OutDisplayScale = ActorToSpawn.AttachTransform.GetScale3D();
				return;
			}
		}
	}
}

UA1Ability_DropItem::UA1Ability_DropItem(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 입력 키로 발동한다.
	ActivationPolicy = ECommonAbilityActivationPolicy::OnInputTriggered;
	ActivationGroup = ECommonAbilityActivationGroup::Independent;

	// 기본 드롭 클래스는 픽업. (메시 지정된 BP 픽업으로 GA 에셋에서 덮어써 사용)
	DropActorClass = AA1Interactable_Pickup::StaticClass();

	SetAssetTags(FGameplayTagContainer(A1GameplayTags::Ability_DropItem));
}

void UA1Ability_DropItem::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (HasAuthority(&ActivationInfo))
	{
		DropItemAuth();
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UA1Ability_DropItem::DropItemAuth()
{
	APawn* Avatar = Cast<APawn>(GetAvatarActorFromActorInfo());
	UWorld* World = GetWorld();
	if (Avatar == nullptr || World == nullptr)
	{
		UE_LOG(A1AbilityDropItemLog, Warning, TEXT("DropItemAuth: Avatar 또는 World가 없어 드롭을 건너뜀."));
		return;
	}

	UInventoryComponent* Inventory = UInventoryComponent::FindInventoryComponent(Avatar);
	if (Inventory == nullptr)
	{
		UE_LOG(A1AbilityDropItemLog, Warning, TEXT("DropItemAuth: InventoryComponent를 찾을 수 없어 드롭을 건너뜀."));
		return;
	}

	// 드롭할 원본 아이템을 결정한다. (장착 아이템이면 해제할 슬롯 태그와 표시 메시·스케일도 함께 받는다)
	FGameplayTag UnequipSlotTag;
	UStaticMesh* DisplayStaticMesh = nullptr;
	USkeletalMesh* DisplaySkeletalMesh = nullptr;
	FVector DisplayScale = FVector::OneVector;
	UItemInstance* ItemToDrop = SelectItemToDropAuth(Avatar, Inventory, UnequipSlotTag, DisplayStaticMesh, DisplaySkeletalMesh, DisplayScale);
	if (ItemToDrop == nullptr)
	{
		UE_LOG(A1AbilityDropItemLog, Log, TEXT("DropItemAuth: 드롭할 %s 아이템이 없음."), bDropEquipped ? TEXT("장착") : TEXT("인벤토리"));
		return;
	}

	// 제거되면 인스턴스가 무효화될 수 있으므로, 스폰에 실어줄 정의·수량을 먼저 확보한다.
	const UItemDefinition* Definition = ItemToDrop->Definition;
	const FInventoryEntry* Entry = Inventory->FindEntry(ItemToDrop);
	const int32 Count = Entry ? Entry->StackCount : 1;

	// 장착 아이템이면 슬롯을 먼저 해제한 뒤 인벤토리에서 제거한다. (해제 → 제거 순서 유지)
	if (UnequipSlotTag.IsValid())
	{
		if (UEquipmentComponent* Equipment = UEquipmentComponent::FindEquipmentComponent(Avatar))
		{
			Equipment->UnequipItemAuth(UnequipSlotTag);
		}
	}

	Inventory->RemoveItemAuth(ItemToDrop);

	// 방금 뺀 아이템 정보·표시 메시·스케일을 실어 픽업 액터를 스폰한다. 주우면 동일 정의·수량이 인벤토리로 복귀한다.
	SpawnDropActorAuth(Definition, Count, DisplayStaticMesh, DisplaySkeletalMesh, DisplayScale);
}

UItemInstance* UA1Ability_DropItem::SelectItemToDropAuth(APawn* Avatar, UInventoryComponent* Inventory, FGameplayTag& OutUnequipSlotTag, UStaticMesh*& OutStaticMesh, USkeletalMesh*& OutSkeletalMesh, FVector& OutDisplayScale) const
{
	OutUnequipSlotTag = FGameplayTag();
	OutStaticMesh = nullptr;
	OutSkeletalMesh = nullptr;
	OutDisplayScale = FVector::OneVector;

	UItemInstance* ItemToDrop = nullptr;

	if (bDropEquipped)
	{
		// 손에 든(활성) 메인 장비의 원본 아이템을 드롭 대상으로 삼는다.
		if (UEquipmentComponent* Equipment = UEquipmentComponent::FindEquipmentComponent(Avatar))
		{
			if (UEquipmentInstance* MainEquip = Equipment->GetActiveMainEquippedItem())
			{
				OutUnequipSlotTag = MainEquip->GetEquipmentSlotTag();
				ItemToDrop = MainEquip->GetSourceItemInstance();
			}
		}
	}
	else
	{
		// 인벤토리에서 장착되지 않은 첫 번째 아이템을 고른다.
		TArray<UItemInstance*> Items;
		Inventory->GetAllItems(Items);
		for (UItemInstance* Item : Items)
		{
			const FInventoryEntry* Entry = Inventory->FindEntry(Item);
			if (Entry && Entry->bEquipment == false)
			{
				ItemToDrop = Item;
				break;
			}
		}
	}

	// 표시 메시·스케일 조회는 장착/인벤토리 경로가 공유하는 단일 헬퍼가 담당한다.
	if (ItemToDrop != nullptr)
	{
		FindDisplayMeshForItem(ItemToDrop, OutStaticMesh, OutSkeletalMesh, OutDisplayScale);
	}

	return ItemToDrop;
}

void UA1Ability_DropItem::SpawnDropActorAuth(const UItemDefinition* ItemDefinition, int32 ItemCount, UStaticMesh* DisplayStaticMesh, USkeletalMesh* DisplaySkeletalMesh, const FVector& DisplayScale)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	UWorld* World = GetWorld();
	if (Avatar == nullptr || World == nullptr)
	{
		UE_LOG(A1AbilityDropItemLog, Warning, TEXT("SpawnDropActorAuth: Avatar 또는 World가 없어 스폰을 건너뜀."));
		return;
	}

	if (DropActorClass == nullptr)
	{
		UE_LOG(A1AbilityDropItemLog, Warning, TEXT("SpawnDropActorAuth: DropActorClass가 지정되지 않아 스폰을 건너뜀."));
		return;
	}

	const FTransform SpawnTransform = ComputeDropTransform(Avatar);

	// 스폰 직후(BeginPlay 전) 아이템 정보를 실어주기 위해 Deferred 스폰을 사용한다.
	// 겹치더라도 항상 스폰하되 가능하면 위치를 살짝 보정한다.
	AA1WorldInteractable* Dropped = World->SpawnActorDeferred<AA1WorldInteractable>(
		DropActorClass, SpawnTransform, Avatar, Avatar->GetInstigator(),
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

	AA1Interactable_Pickup* Pickup = Cast<AA1Interactable_Pickup>(Dropped);
	if (Pickup != nullptr)
	{
		Pickup->InitializePickupDataAuth(ItemDefinition, ItemCount);
	}

	if (Dropped)
	{
		Dropped->FinishSpawning(SpawnTransform);
	}

	// 표시 메시·스케일은 Mesh 컴포넌트 등록(FinishSpawning) 이후에 반영한다. (서버 즉시 반영 + 클라 복제)
	if (Pickup != nullptr)
	{
		Pickup->SetDisplayMeshAuth(DisplayStaticMesh, DisplaySkeletalMesh, DisplayScale);
	}

	UE_LOG(A1AbilityDropItemLog, Log, TEXT("드롭 스폰: %s (by %s) Item=%s x%d StaticMesh=%s SkeletalMesh=%s Scale=%s at %s"),
		*GetNameSafe(Dropped), *GetNameSafe(Avatar), *GetNameSafe(ItemDefinition), ItemCount,
		*GetNameSafe(DisplayStaticMesh), *GetNameSafe(DisplaySkeletalMesh), *DisplayScale.ToCompactString(), *SpawnTransform.GetLocation().ToCompactString());
}

FTransform UA1Ability_DropItem::ComputeDropTransform(const AActor* Avatar) const
{
	const FVector Forward = Avatar->GetActorForwardVector();
	FVector Location = Avatar->GetActorLocation() + Forward * DropForwardDistance;

	// 스폰 지점에서 아래로 라인 트레이스해 지면에 붙인다.
	if (const UWorld* World = GetWorld())
	{
		const FVector TraceStart = Location + FVector(0.f, 0.f, GroundTraceDistance * 0.5f);
		const FVector TraceEnd = Location - FVector(0.f, 0.f, GroundTraceDistance);

		FCollisionQueryParams Params(SCENE_QUERY_STAT(A1Ability_DropItem_Ground), false);
		Params.AddIgnoredActor(Avatar);

		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
		{
			Location = Hit.ImpactPoint;
		}
	}

	// 아바타의 Yaw만 반영해 눕지 않도록 회전을 구성한다.
	const FRotator Rotation(0.f, Avatar->GetActorRotation().Yaw, 0.f);
	return FTransform(Rotation, Location);
}
