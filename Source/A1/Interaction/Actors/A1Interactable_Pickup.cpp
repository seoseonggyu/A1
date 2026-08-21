#include "A1Interactable_Pickup.h"

#include "A1GameplayTags.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Pawn.h"
#include "Inventory/InventoryComponent.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Interactable_Pickup)

DEFINE_LOG_CATEGORY(A1InteractablePickupLog);

AA1Interactable_Pickup::AA1Interactable_Pickup(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InteractionTitle = NSLOCTEXT("A1Interaction", "Pickup", "줍기");
	HighlightStencil = 3;
	InteractEventTag = A1GameplayTags::GameplayEvent_Interact_Pickup;
	bConsumeOnUse = true;

	// SkeletalMesh 원본을 가진 아이템을 드롭할 때만 사용하는 보조 표시 컴포넌트. 기본은 숨김.
	DisplaySkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DisplaySkeletalMeshComponent"));
	DisplaySkeletalMeshComponent->SetupAttachment(Mesh);
	DisplaySkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DisplaySkeletalMeshComponent->SetVisibility(false);

	// 표시 메시 종류와 무관하게 항상 콜리전을 제공하는 스캔/커서 트레이스 감지 전용 콜리전.
	InteractionCollision = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionCollision"));
	InteractionCollision->SetupAttachment(Mesh);
	InteractionCollision->SetCollisionProfileName(TEXT("A1Interactable"));
	InteractionCollision->SetSphereRadius(50.f);
}

void AA1Interactable_Pickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, DisplayStaticMesh, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, DisplaySkeletalMesh, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, DisplayScale, Params);
}

void AA1Interactable_Pickup::GetHighlightComponents(TArray<UPrimitiveComponent*>& OutComponents) const
{
	// SkeletalMesh로 표시 중이면(실제로 보이는 컴포넌트) 그쪽에 외곽선을 적용해야 눈에 보인다.
	if (DisplaySkeletalMesh != nullptr && DisplaySkeletalMeshComponent != nullptr)
	{
		OutComponents.Add(DisplaySkeletalMeshComponent);
		return;
	}

	Super::GetHighlightComponents(OutComponents);
}

void AA1Interactable_Pickup::InitializePickupDataAuth(const UItemDefinition* InItemDefinition, int32 InItemCount)
{
	if (HasAuthority() == false)
		return;

	ItemDefinition = InItemDefinition;
	ItemCount = FMath::Max(1, InItemCount);
}

void AA1Interactable_Pickup::SetDisplayMeshAuth(UStaticMesh* InStaticMesh, USkeletalMesh* InSkeletalMesh, const FVector& InDisplayScale)
{
	if (HasAuthority() == false)
		return;

	DisplayStaticMesh = InStaticMesh;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, DisplayStaticMesh, this);

	DisplaySkeletalMesh = InSkeletalMesh;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, DisplaySkeletalMesh, this);

	DisplayScale = InDisplayScale;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, DisplayScale, this);

	// 서버 자신에게도 즉시 반영한다. (클라는 각 OnRep에서 반영)
	OnRep_DisplayStaticMesh();
	OnRep_DisplaySkeletalMesh();
	OnRep_DisplayScale();
}

void AA1Interactable_Pickup::OnRep_DisplayStaticMesh()
{
	if (Mesh != nullptr && DisplayStaticMesh != nullptr)
	{
		Mesh->SetStaticMesh(DisplayStaticMesh);
		Mesh->SetRelativeScale3D(DisplayScale);
	}
}

void AA1Interactable_Pickup::OnRep_DisplaySkeletalMesh()
{
	if (DisplaySkeletalMeshComponent == nullptr)
	{
		return;
	}

	DisplaySkeletalMeshComponent->SetSkeletalMesh(DisplaySkeletalMesh);
	DisplaySkeletalMeshComponent->SetVisibility(DisplaySkeletalMesh != nullptr);
	DisplaySkeletalMeshComponent->SetRelativeScale3D(DisplayScale);
}

void AA1Interactable_Pickup::OnRep_DisplayScale()
{
	// 메시(DisplayStaticMesh/DisplaySkeletalMesh)와 스케일(DisplayScale)의 복제 순서는 보장되지 않으므로,
	// 어느 쪽이 나중에 도착하든 최종적으로 두 컴포넌트 모두에 다시 반영해 일치시킨다.
	if (Mesh != nullptr)
	{
		Mesh->SetRelativeScale3D(DisplayScale);
	}

	if (DisplaySkeletalMeshComponent != nullptr)
	{
		DisplaySkeletalMeshComponent->SetRelativeScale3D(DisplayScale);
	}
}

void AA1Interactable_Pickup::OnInteractAuth(AActor* Interactor)
{
	if (HasAuthority() == false)
		return;

	Super::OnInteractAuth(Interactor);

	if (ItemDefinition != nullptr)
	{
		if (UInventoryComponent* Inventory = UInventoryComponent::FindInventoryComponent(Cast<APawn>(Interactor)))
		{
			// 결과(성공/공간 부족 등)는 인벤토리 쪽에서 로깅한다. 여기서는 대상 액터 제거만 보장한다.
			Inventory->AddItemAuthCoroutine(ItemDefinition, ItemCount);
		}
		else
		{
			UE_LOG(A1InteractablePickupLog, Warning, TEXT("OnInteractAuth: %s의 InventoryComponent를 찾을 수 없어 아이템을 지급하지 못함."), *GetNameSafe(Interactor));
		}
	}

	Destroy();
}
