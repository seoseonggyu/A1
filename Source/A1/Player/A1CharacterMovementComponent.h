// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "A1CharacterMovementComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1CharacterMovementComponentLog, Log, All);

/**
 * UA1CharacterMovementComponent
 *
 * GetMaxSpeed()에 소유자 ASC의 UA1VitalSet::MoveSpeedMultiplier를 곱해 반환한다.
 * 슬로우 등 이동속도 배율 효과를 GameplayEffect 하나로 서버/모든 클라에 동일하게 반영하기 위함이며,
 * Sprint처럼 값 자체(MaxWalkSpeed)를 바꾸는 로컬 방식과는 달리 항상 배율로만 곱해진다.
 */
UCLASS()
class A1_API UA1CharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UA1CharacterMovementComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual float GetMaxSpeed() const override;
};
