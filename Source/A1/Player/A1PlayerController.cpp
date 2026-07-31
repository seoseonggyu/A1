// Copyright Epic Games, Inc. All Rights Reserved.

#include "A1PlayerController.h"
#include "Components/InputComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/EngineTypes.h"
#include "Interaction/A1Interactable.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Physics/A1CollisionChannels.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(A1PlayerController)

DEFINE_LOG_CATEGORY(A1PlayerControllerLog);

AA1PlayerController::AA1PlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 탑다운 커서 조작을 위해 마우스 커서와 클릭/호버 이벤트를 활성화한다.
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

void AA1PlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	// 커서 호버는 순수 로컬 연출이므로 소유 클라에서만 갱신한다. (데디케이티드 서버 불필요)
	if (IsLocalController())
	{
		UpdateInteractionHoverLocal();
	}

	Super::PostProcessInput(DeltaTime, bGamePaused);
}


void AA1PlayerController::OnInteractionRightClickLocal()
{
	AActor* Hovered = HoveredInteractable.Get();
	if (Hovered == nullptr)
		return;

	// TODO: 실제 상호작용 발동(UA1Ability_Interact 활성화)으로 교체.
	UE_LOG(A1PlayerControllerLog, Log, TEXT("[TODO] 상호작용 우클릭: %s"), *Hovered->GetName());
}

void AA1PlayerController::UpdateInteractionHoverLocal()
{
	AActor* NewHovered = nullptr;

	FHitResult HitResult;
	const ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(A1_TraceChannel_Interaction);
	if (GetHitResultUnderCursorByChannel(TraceType, false, HitResult))
	{
		AActor* HitActor = HitResult.GetActor();
		// IA1Interactable을 구현하고, 현재 상호작용 가능한 대상만 하이라이트한다.
		if (Cast<IA1Interactable>(HitActor) != nullptr && HitActor != Cast<AActor>(GetPawn()))
		{
			FA1InteractionQuery Query;
			Query.RequestingAvatar = Cast<AActor>(GetPawn());
			Query.RequestingController = this;

			if (Cast<IA1Interactable>(HitActor)->CanInteract(Query))
			{
				NewHovered = HitActor;
			}
		}
	}

	if (NewHovered == HoveredInteractable.Get())
		return;

	// 이전 대상 하이라이트 해제 후 새 대상 하이라이트.
	SetInteractableHighlightLocal(HoveredInteractable.Get(), false);
	HoveredInteractable = NewHovered;
	SetInteractableHighlightLocal(NewHovered, true);
}

void AA1PlayerController::SetInteractableHighlightLocal(AActor* InteractableActor, bool bHighlight)
{
	IA1Interactable* Interactable = Cast<IA1Interactable>(InteractableActor);
	if (Interactable == nullptr)
		return;

	int32 Stencil = 1;
	if (bHighlight)
	{
		// 대상이 제공하는 스텐실 값으로 외곽선 색을 구분한다.
		FA1InteractionQuery Query;
		Query.RequestingAvatar = Cast<AActor>(GetPawn());
		Query.RequestingController = this;

		TArray<FA1InteractionOption> Options;
		Interactable->GatherInteractionOptions(Query, Options);
		if (Options.Num() > 0)
		{
			Stencil = Options[0].HighlightStencil;
		}
	}

	TArray<UPrimitiveComponent*> Components;
	Interactable->GetHighlightComponents(Components);
	for (UPrimitiveComponent* Component : Components)
	{
		if (Component == nullptr)
			continue;

		Component->SetRenderCustomDepth(bHighlight);
		if (bHighlight)
		{
			Component->SetCustomDepthStencilValue(Stencil);
		}
	}
}

void AA1PlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
}
