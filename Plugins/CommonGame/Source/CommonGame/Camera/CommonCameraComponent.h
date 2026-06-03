
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
 *	이 플러그인에서 사용하는 기본 카메라 구성 요소 클래스입니다.
 */
UCLASS()
class UCommonCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public:
	UCommonCameraComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category = "Common|Camera")
	static UCommonCameraComponent* FindCameraComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UCommonCameraComponent>() : nullptr); }

	virtual AActor* GetTargetActor() const { return GetOwner(); }

	FCommonCameraModeDelegate DetermineCameraModeDelegate;

	void AddFieldOfViewOffset(float FovOffset) { FieldOfViewOffset += FovOffset; }

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