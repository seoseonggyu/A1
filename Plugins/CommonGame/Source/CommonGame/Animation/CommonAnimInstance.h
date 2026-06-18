// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CommonAnimInstance.generated.h"

class UCharacterMovementComponent;

DECLARE_LOG_CATEGORY_EXTERN(CommonAnimInstanceLog, Log, All);


UCLASS()
class COMMONGAME_API UCommonAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UCommonAnimInstance(const FObjectInitializer& ObjectInitializer);

	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

protected:
	//-----------------------------------------------------------------------------
	// Thread Safe 데이터 업데이트
	//-----------------------------------------------------------------------------

	void UpdateLocationData(float DeltaSeconds);
	void UpdateMovementStates();

protected:

	//-----------------------------------------------------------------------------
	// 캐시된 레퍼런스
	//-----------------------------------------------------------------------------

	UPROPERTY(Transient, BlueprintReadOnly, Category = "References")
	TObjectPtr<ACharacter> CachedCharacter;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "References")
	TObjectPtr<UCharacterMovementComponent> MovementComponent;

	//-----------------------------------------------------------------------------
	// Thread Safe 애니메이션 데이터 - Location
	//-----------------------------------------------------------------------------

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Animation Data|Location")
	FVector3f Velocity = FVector3f::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Animation Data|Location")
	float GroundSpeed = 0.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Animation Data|Location")
	float Direction = 0.f;

	//-----------------------------------------------------------------------------
	// Thread Safe 애니메이션 데이터 - States
	//-----------------------------------------------------------------------------

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Animation Data|States")
	bool bIsMoving = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Animation Data|States")
	bool bIsFalling = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Animation Data|States")
	bool bIsJumping = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Animation Data|States")
	bool bIsCrouching = false;


	//-----------------------------------------------------------------------------
	// 애니메이션 데이터 - Settings
	//-----------------------------------------------------------------------------
	
	float MovingSpeedThreshold = 3.f;
};