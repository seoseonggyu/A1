// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonCameraMode.h"
#include "CommonCameraMode_FreeFly.generated.h"

/**
 * 자유 시점(자유 비행) 카메라 모드
 *
 * 캐릭터를 따라가지 않고 자체 위치/회전 상태를 유지한다. 마우스로 시점을 회전하고,
 * 바라보는 방향(피치 포함) 기준 전/후/좌/우로 자유롭게 날아다닐 수 있다.
 * 활성화 시점의 카메라 위치/회전을 그대로 이어받아 시작하므로 다른 모드에서 전환해도 튀지 않는다.
 */
UCLASS(Blueprintable)
class COMMONGAME_API UCommonCameraMode_FreeFly : public UCommonCameraMode
{
	GENERATED_BODY()

public:
	UCommonCameraMode_FreeFly();

	virtual void OnActivation() override;

	/** 시점 입력(마우스 등)을 누적한다. UpdateView에서 소비된다. */
	void AddLookInput(const FVector2D& LookAxis);

	/** 이동 입력(WASD 등)을 누적한다. UpdateView에서 소비된다. */
	void AddMoveInput(const FVector2D& MoveAxis);

protected:
	virtual void UpdateView(float DeltaTime) override;

protected:
	/** 초당 이동 속도(cm/s) */
	UPROPERTY(EditDefaultsOnly, Category = "FreeFly")
	float FlySpeed;

	/** 시점 입력 감도 배율 */
	UPROPERTY(EditDefaultsOnly, Category = "FreeFly")
	float LookSensitivity;

private:
	/** 자유 시점 자체 위치(캐릭터 위치와 무관하게 누적됨) */
	FVector FreeLocation;

	/** 자유 시점 자체 회전(캐릭터 회전과 무관하게 누적됨) */
	FRotator FreeRotation;

	/** 이번 프레임에 누적된 시점 입력. UpdateView에서 소비 후 초기화된다. */
	FVector2D PendingLookInput;

	/** 이번 프레임에 누적된 이동 입력. UpdateView에서 소비 후 초기화된다. */
	FVector2D PendingMoveInput;
};
