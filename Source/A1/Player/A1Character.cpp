// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/A1Character.h"

#include "Actors/A1ArmorBase.h"
#include "Camera/CommonCameraComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Cosmetic/A1CosmeticManagerComponent.h"
#include "Physics/A1CollisionChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Character)

DEFINE_LOG_CATEGORY(A1CharacterLog);

AA1Character::AA1Character(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FRotator initRotator = FRotator(0.0f, -90.0f, 0.0f);

	CameraComponent->SetRelativeRotation(initRotator);

	UArrowComponent* arrComp = GetArrowComponent();
	arrComp->SetRelativeRotation(initRotator);

	USkeletalMeshComponent* MeshComp = GetMesh();
	MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -89.0f));
	MeshComp->SetRelativeRotation(initRotator);
	MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	// 상호작용 커서 트레이스 채널에만 추가로 반응한다. 기존 Pawn 콜리전(캡슐)에는 영향 없음.
	GetCapsuleComponent()->SetCollisionResponseToChannel(A1_TraceChannel_Interaction, ECR_Block);
}

void AA1Character::GetHighlightComponents(TArray<UPrimitiveComponent*>& OutComponents) const
{
	if ( UA1CosmeticManagerComponent* CosmeticCmp = UA1CosmeticManagerComponent::FindCosmeticManagerComponent(this))
	{
		TArray<TObjectPtr<UChildActorComponent>> Slots = CosmeticCmp->GetCosmeticSlots();

		for (UChildActorComponent* ChildActorComponent : Slots)
		{
			if (!ChildActorComponent)
			{
				continue;
			}

			if (AA1ArmorBase* ArmorActor = Cast<AA1ArmorBase>(ChildActorComponent->GetChildActor()))
			{
				if (UPrimitiveComponent* MeshComponent = ArmorActor->GetMesh())
				{
					OutComponents.Add(MeshComponent);
				}
			}
		}
	}


	// TODO:
}

void AA1Character::GatherInteractionOptions(const FA1InteractionQuery& Query, TArray<FA1InteractionOption>& OutOptions) const
{
	FA1InteractionOption Option;
	Option.Interactable = TScriptInterface<IA1Interactable>(const_cast<AA1Character*>(this));
	Option.Title = FText::FromString(GetActorNameOrLabel());
	// 캐릭터 전용 외곽선 스텐실 값. World Interactable 예시들과 겹치지 않도록 5를 사용한다
	// (Door=2, Pickup=3, Corpse=4, WorldInteractable 기본=1).
	Option.HighlightStencil = 255;
	OutOptions.Add(Option);
}

bool AA1Character::CanInteract(const FA1InteractionQuery& Query) const
{
	return true;
}
