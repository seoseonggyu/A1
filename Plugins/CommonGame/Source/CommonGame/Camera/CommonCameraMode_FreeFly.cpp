// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonCameraMode_FreeFly.h"
#include "CommonCameraComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonCameraMode_FreeFly)

UCommonCameraMode_FreeFly::UCommonCameraMode_FreeFly()
{
	FlySpeed = 1000.f;
	LookSensitivity = 1.0f;

	FreeLocation = FVector::ZeroVector;
	FreeRotation = FRotator::ZeroRotator;
	PendingLookInput = FVector2D::ZeroVector;
	PendingMoveInput = FVector2D::ZeroVector;
}

void UCommonCameraMode_FreeFly::OnActivation()
{
	Super::OnActivation();

	// 직전 카메라 위치/회전을 그대로 이어받아 전환 시 튀지 않게 한다.
	const UCommonCameraComponent* CommonCameraComponent = GetCommonCameraComponent();
	FreeLocation = CommonCameraComponent->GetComponentLocation();
	FreeRotation = CommonCameraComponent->GetComponentRotation();

	PendingLookInput = FVector2D::ZeroVector;
	PendingMoveInput = FVector2D::ZeroVector;
}

void UCommonCameraMode_FreeFly::AddLookInput(const FVector2D& LookAxis)
{
	PendingLookInput += LookAxis;
}

void UCommonCameraMode_FreeFly::AddMoveInput(const FVector2D& MoveAxis)
{
	PendingMoveInput += MoveAxis;
}

void UCommonCameraMode_FreeFly::UpdateView(float DeltaTime)
{
	// LookAxis.X: 마우스 좌우(Yaw), LookAxis.Y: 마우스 상하(Pitch, 위로 올리면 위를 보도록 부호 반전)
	FreeRotation.Yaw += PendingLookInput.X * LookSensitivity;
	FreeRotation.Pitch = FMath::ClampAngle(FreeRotation.Pitch + PendingLookInput.Y * LookSensitivity, ViewPitchMin, ViewPitchMax);
	FreeRotation.Roll = 0.f;

	// MoveAxis.X: 전/후, MoveAxis.Y: 좌/우. Forward는 피치까지 포함하므로 바라보는 방향으로 그대로 날아간다.
	const FRotationMatrix RotationMatrix(FreeRotation);
	const FVector Forward = RotationMatrix.GetUnitAxis(EAxis::X);
	const FVector Right = RotationMatrix.GetUnitAxis(EAxis::Y);

	FreeLocation += (Forward * PendingMoveInput.X + Right * PendingMoveInput.Y) * FlySpeed * DeltaTime;

	PendingLookInput = FVector2D::ZeroVector;
	PendingMoveInput = FVector2D::ZeroVector;

	View.Location = FreeLocation;
	View.Rotation = FreeRotation;
	View.ControlRotation = View.Rotation;
	View.FieldOfView = FieldOfView;
}
