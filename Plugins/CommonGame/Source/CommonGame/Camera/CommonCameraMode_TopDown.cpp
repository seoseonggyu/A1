// Fill out your copyright notice in the Description page of Project Settings.


#include "CommonCameraMode_TopDown.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonCameraMode_TopDown)

UCommonCameraMode_TopDown::UCommonCameraMode_TopDown()
{
}

void UCommonCameraMode_TopDown::UpdateView(float DeltaTime)
{
	FVector PivotLocation = GetPivotLocation();
	//FRotator PivotRotation = GetPivotRotation();

	//PivotRotation.Pitch = FMath::ClampAngle(PivotRotation.Pitch, ViewPitchMin, ViewPitchMax);

	//PivotRotation.Pitch += 50.f;

	PivotLocation.Z += 1000.f;
	PivotLocation.X -= 500.f;


	FRotator PivotRotation;
	PivotRotation.Pitch = -60.f; // 위에서 아래로 60도 내려다보기
	PivotRotation.Yaw = 0.f;     // 필요에 따라 회전 (캐릭터 기준 북쪽, 동쪽 등)
	PivotRotation.Roll = 0.f;

	View.Location = PivotLocation;
	View.Rotation = PivotRotation;
	View.ControlRotation = View.Rotation;
	View.FieldOfView = FieldOfView;
}
