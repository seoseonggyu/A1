// Copyright Epic Games, Inc. All Rights Reserved.

#include "Equipment/EquipmentInstance.h"
#include "Game/CommonCharacter.h"
#include "Inventory/InventoryComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(EquipmentInstance)

ACommonCharacter* UEquipmentInstance::GetOwningCharacter() const
{
	return Cast<ACommonCharacter>(GetOuter());
}

bool UEquipmentInstance::HasAuthority() const
{
	if (AActor* Owner = GetOwningCharacter())
	{
		return Owner->HasAuthority();
	}
	
	return false;
}

bool UEquipmentInstance::IsLocallyControlled() const
{
	if (ACommonCharacter* Character = GetOwningCharacter())
	{
		return Character->IsLocallyControlled();
	}

	return false;
}

UItemInstance* UEquipmentInstance::GetSourceItemInstance() const
{
	if (SourceItemId == INDEX_NONE)
	{
		return nullptr;
	}

	if (UInventoryComponent* Inventory = UInventoryComponent::FindInventoryComponent(GetOwningCharacter()))
	{
		return Inventory->FindItemById(SourceItemId);
	}

	return nullptr;
}

void UEquipmentInstance::ActivateEquipment()
{
	if (bEquipmentActive)
	{
		return;
	}
	bEquipmentActive = true;

	SpawnEquipmentActors();
	OnEquipped();
}

void UEquipmentInstance::DeactivateEquipment()
{
	if (!bEquipmentActive)
	{
		return;
	}
	bEquipmentActive = false;

	OnUnequipped();
	DestroyEquipmentActors();
}

void UEquipmentInstance::OnEquipped()
{
	if (!Definition)
	{
		return;
	}

	// Definition에서 Fragment 복사합니다
	Fragments = Definition->Fragments;

	// 각 Fragment의 OnEquipped 호출합니다
	for (TInstancedStruct<FEquipmentFragment>& Entry : Fragments)
	{
		if (FEquipmentFragment* Fragment = Entry.GetMutablePtr<FEquipmentFragment>())
		{
			Fragment->OnEquipped(this);
		}
	}
}

void UEquipmentInstance::OnUnequipped()
{
	// 각 Fragment의 OnUnequipped 호출합니다
	for (TInstancedStruct<FEquipmentFragment>& Entry : Fragments)
	{
		if (FEquipmentFragment* Fragment = Entry.GetMutablePtr<FEquipmentFragment>())
		{
			Fragment->OnUnequipped(this);
		}
	}
	
	Fragments.Empty();
}

void UEquipmentInstance::SpawnEquipmentActors()
{
	if (!Definition)
	{
		return;
	}

	for (const FEquipmentActorToSpawn& ActorToSpawn : Definition->ActorsToSpawn)
	{
		if (AActor* Actor = SpawnActor(ActorToSpawn))
		{
			SpawnedActors.Add(Actor);
		}
	}
}

void UEquipmentInstance::DestroyEquipmentActors()
{
	for (AActor* Actor : SpawnedActors)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}
	SpawnedActors.Empty();
}

AActor* UEquipmentInstance::SpawnActor(const FEquipmentActorToSpawn& ActorToSpawn) const
{
	if (!ActorToSpawn.ActorClass)
	{
		return nullptr;
	}

	ACommonCharacter* OwningCharacter = GetOwningCharacter();
	if (!OwningCharacter)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwningCharacter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* NewActor = World->SpawnActor<AActor>(ActorToSpawn.ActorClass, SpawnParams);
	if (NewActor)
	{
		if (!ActorToSpawn.AttachSocket.IsNone())
		{

			USkeletalMeshComponent* TargetMesh = OwningCharacter->GetMesh();
			NewActor->AttachToComponent(TargetMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, ActorToSpawn.AttachSocket);
		}
		else
		{
			NewActor->AttachToActor(OwningCharacter, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}

		NewActor->SetActorRelativeTransform(ActorToSpawn.AttachTransform);
	}

	return NewActor;
}
