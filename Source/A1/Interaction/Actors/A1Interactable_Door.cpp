#include "A1Interactable_Door.h"

#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Interactable_Door)

AA1Interactable_Door::AA1Interactable_Door(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InteractionTitle = NSLOCTEXT("A1Interaction", "Door", "문");
	HighlightStencil = 2;
	InteractEventTag = FGameplayTag();
	bConsumeOnUse = false;
}

void AA1Interactable_Door::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, bIsOpen, Params);
}

void AA1Interactable_Door::OnInteractAuth(AActor* Interactor)
{
	if (HasAuthority() == false)
		return;

	bIsOpen = !bIsOpen;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, bIsOpen, this);
	ApplyDoorVisual(); // 서버에서도 즉시 반영(콜리전/상태 일관성)

	Super::OnInteractAuth(Interactor); // 로그 + K2 훅
}

void AA1Interactable_Door::OnRep_bIsOpen()
{
	ApplyDoorVisual();
}

void AA1Interactable_Door::ApplyDoorVisual()
{
	if (Mesh == nullptr)
		return;

	FRotator Rot = Mesh->GetRelativeRotation();
	Rot.Yaw = bIsOpen ? OpenYaw : ClosedYaw;
	Mesh->SetRelativeRotation(Rot);
}
