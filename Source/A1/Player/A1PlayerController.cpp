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
	// 커서 호버 하이라이트는 주변 스캔 어빌리티(UA1Ability_Interact_Scan)로 대체되었다.
	// 하이라이트 중복/충돌을 막기 위해 여기서는 갱신하지 않는다. (코드는 참고용으로 남겨둠)
	// if (IsLocalController())
	// {
	// 	UpdateInteractionHoverLocal();
	// }

	Super::PostProcessInput(DeltaTime, bGamePaused);
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

void AA1PlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (IsLocalController())
	{
		UpdateCursorHit();
	}
}

void AA1PlayerController::UpdateCursorHit()
{
	FHitResult HitResult;

	if (GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	{
		CachedCursorHitLocation = HitResult.Location;
	}
}
