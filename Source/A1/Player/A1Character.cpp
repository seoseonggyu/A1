// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/A1Character.h"

#include "A1GameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/A1VitalSet.h"
#include "Actors/A1ArmorBase.h"
#include "Camera/CommonCameraComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Cosmetic/A1CosmeticManagerComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
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
	const UA1CosmeticManagerComponent* CosmeticCmp = UA1CosmeticManagerComponent::FindCosmeticManagerComponent(this);
	if (CosmeticCmp == nullptr)
	{
		return;
	}

	const TArray<TObjectPtr<UChildActorComponent>>& Slots = CosmeticCmp->GetCosmeticSlots();

	for (const TObjectPtr<UChildActorComponent>& ChildActorComponent : Slots)
	{
		if (ChildActorComponent == nullptr)
		{
			continue;
		}

		const AA1ArmorBase* ArmorActor = Cast<AA1ArmorBase>(ChildActorComponent->GetChildActor());
		if (ArmorActor == nullptr)
		{
			continue;
		}

		if (USkeletalMeshComponent* ArmorMesh = ArmorActor->GetMesh())
		{
			OutComponents.Add(ArmorMesh);
		}
	}
}

void AA1Character::GatherInteractionOptions(const FA1InteractionQuery& Query, TArray<FA1InteractionOption>& OutOptions) const
{
	FA1InteractionOption Option;
	Option.Interactable = TScriptInterface<IA1Interactable>(const_cast<AA1Character*>(this));
	Option.Title = FText::FromString(GetActorNameOrLabel());
	// CanInteract가 사망 상태만 통과시키므로, 여기 도달하는 대상은 항상 시체다. (Corpse 스텐실 값 사용)
	Option.HighlightStencil = 4;
	Option.InteractEventTag = A1GameplayTags::GameplayEvent_Interact_Player;
	OutOptions.Add(Option);
}

bool AA1Character::CanInteract(const FA1InteractionQuery& Query) const
{
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (ASC == nullptr)
	{
		return false;
	}

	bool bFound = false;
	const float Health = ASC->GetGameplayAttributeValue(UA1VitalSet::GetHealthAttribute(), bFound);
	if (bFound == false)
	{
		return false;
	}

	return Health <= 0.f;
}

void AA1Character::HandleDeathAuth()
{
	if (HasAuthority() == false) return;

	UE_LOG(A1CharacterLog, Log, TEXT("%s 사망"), *GetName());
}
