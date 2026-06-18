// Copyright Epic Games, Inc. All Rights Reserved.

#include "Animation/CommonAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "Kismet\KismetMathLibrary.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonAnimInstance)

DEFINE_LOG_CATEGORY(CommonAnimInstanceLog);

UCommonAnimInstance::UCommonAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UCommonAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	CachedCharacter = Cast<ACharacter>(GetOwningActor());
	if (!CachedCharacter)
	{
		return;
	}
	MovementComponent = CachedCharacter->GetCharacterMovement();
}

void UCommonAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	// 주의: 워커 스레드에서 실행되므로 외부 오브젝트의 값을 수정하면 안됩니다
	// 읽기만 허용되며, AnimInstance 내부 변수에만 값을 저장해야 합니다

	if (!MovementComponent)
	{
		return;
	}

	UpdateLocationData(DeltaSeconds);
	UpdateMovementStates();

}

void UCommonAnimInstance::UpdateLocationData(float DeltaSeconds)
{
	Velocity = FVector3f(MovementComponent->Velocity);
	GroundSpeed = Velocity.Size2D();

	Direction = UKismetAnimationLibrary::CalculateDirection(
		MovementComponent->Velocity,
		CachedCharacter->GetActorRotation()
	);

}

void UCommonAnimInstance::UpdateMovementStates()
{
	bool isAccelZero = UKismetMathLibrary::Vector_IsZero(MovementComponent->GetCurrentAcceleration());
	
	bIsMoving = (GroundSpeed > MovingSpeedThreshold && !isAccelZero);
	bIsFalling = MovementComponent->IsFalling();
	bIsJumping = bIsFalling && Velocity.Z > 0.f;
	bIsCrouching = MovementComponent->IsCrouching();

}
