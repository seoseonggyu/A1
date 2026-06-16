#include "A1CosmeticManagerComponent.h"
#include "GameFramework/Character.h"
#include "Actors/A1ArmorBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1CosmeticManagerComponent)

UA1CosmeticManagerComponent::UA1CosmeticManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UA1CosmeticManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeManager();
}

void UA1CosmeticManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (UChildActorComponent* CosmeticSlot : CosmeticSlots)
	{
		if (CosmeticSlot)
		{
			CosmeticSlot->DestroyComponent();
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UA1CosmeticManagerComponent::InitializeManager()
{
	if (bInitialized)
		return;

	bInitialized = true;

	const int32 ArmorTypeCount = (int32)EArmorType::Count;
	CosmeticSlots.SetNumZeroed(ArmorTypeCount);

	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (Character->IsNetMode(NM_DedicatedServer) == false)
		{
			for (int i = 0; i < ArmorTypeCount; ++i)
			{
				CosmeticSlots[i] = SpawnCosmeticSlotActor(InitialCosmetics[i]);
			}
		}
	}

}

UChildActorComponent* UA1CosmeticManagerComponent::SpawnCosmeticSlotActor(TSoftObjectPtr<USkeletalMesh> InDefaultMesh)
{
	UChildActorComponent* CosmeticComponent = nullptr;

	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		USceneComponent* ComponentToAttachTo = Character->GetMesh();
		CosmeticComponent = NewObject<UChildActorComponent>(Character);
		CosmeticComponent->SetupAttachment(ComponentToAttachTo);
		CosmeticComponent->SetChildActorClass(CosmeticSlotClass);
		CosmeticComponent->RegisterComponent();

		if (AA1ArmorBase* SpawnedActor = Cast<AA1ArmorBase>(CosmeticComponent->GetChildActor()))
		{
			if (USceneComponent* SpawnedRootComponent = SpawnedActor->GetRootComponent())
			{
				SpawnedRootComponent->AddTickPrerequisiteComponent(ComponentToAttachTo);
			}

			SpawnedActor->InitializeActor(InDefaultMesh);
		}
	}

	return CosmeticComponent;
}

