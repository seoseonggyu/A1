
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Camera/CameraComponent.h"
#include "GameFramework/Actor.h"

#include "CommonCameraComponent.generated.h"

class UCommonCameraMode;
class UCommonCameraModeStack;
struct FGameplayTag;


DECLARE_DELEGATE_RetVal(TSubclassOf<UCommonCameraMode>, FCommonCameraModeDelegate);

/**
 * UCommonCameraComponent
 *
 *	�� �÷����ο��� ����ϴ� �⺻ ī�޶� ���� ��� Ŭ�����Դϴ�.
 */
UCLASS()
class COMMONGAME_API UCommonCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public:
	UCommonCameraComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category = "Common|Camera")
	static UCommonCameraComponent* FindCameraComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UCommonCameraComponent>() : nullptr); }

	virtual AActor* GetTargetActor() const { return GetOwner(); }

	FCommonCameraModeDelegate DetermineCameraModeDelegate;

	void AddFieldOfViewOffset(float FovOffset) { FieldOfViewOffset += FovOffset; }

	/** 현재 스택 맨 위(가장 우선순위 높은) 카메라 모드 인스턴스를 반환한다. 없으면 nullptr. */
	UCommonCameraMode* GetTopCameraMode() const;

	void GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const;

protected:

	virtual void OnRegister() override;
	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;

	virtual void UpdateCameraModes();

protected:

	UPROPERTY()
	TObjectPtr<UCommonCameraModeStack> CameraModeStack;

	float FieldOfViewOffset;
};