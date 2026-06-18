// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/A1Character.h"
#include "Camera/CommonCameraComponent.h"
#include "Components/ArrowComponent.h"

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
}
