// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "CommonCameraMode.generated.h"

class UCommonCameraComponent;

/**
 * 카메라 모드 블렌드 기능
 *
 *	
 */
UENUM(BlueprintType)
enum class ECommonCameraModeBlendFunction : uint8
{
	Linear,
	EaseIn,
	EaseOut,
	EaseInOut,
	
	COUNT	UMETA(Hidden)
};

/**
 * 카메라 모드 뷰
 *
 *	
 */
struct FCommonCameraModeView
{
public:

	FCommonCameraModeView();

	void Blend(const FCommonCameraModeView& Other, float OtherWeight);

public:

	FVector Location;
	FRotator Rotation;
	FRotator ControlRotation;
	float FieldOfView;
};

/**
 * 카메라 모드의 베이스 클래스
 *
 *
 */
UCLASS(Abstract, Blueprintable)
class COMMONGAME_API UCommonCameraMode : public UObject
{
	GENERATED_BODY()

public:
	UCommonCameraMode();

	UCommonCameraComponent* GetCommonCameraComponent() const;

	virtual UWorld* GetWorld() const override;

	AActor* GetTargetActor() const;

	const FCommonCameraModeView& GetCameraModeView() const { return View; }

	virtual void OnActivation() {};

	virtual void OnDeactivation() {};

	void UpdateCameraMode(float DeltaTime);

	float GetBlendTime() const { return BlendTime; }
	float GetBlendWeight() const { return BlendWeight; }
	void SetBlendWeight(float Weight);

	FGameplayTag GetCameraTypeTag() const
	{
		return CameraTypeTag;
	}

protected:

	virtual FVector GetPivotLocation() const;
	virtual FRotator GetPivotRotation() const;

	virtual void UpdateView(float DeltaTime);
	virtual void UpdateBlending(float DeltaTime);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	FGameplayTag CameraTypeTag;

	FCommonCameraModeView View;

	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "5.0", UIMax = "170", ClampMin = "5.0", ClampMax = "170.0"))
	float FieldOfView;

	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "-89.9", UIMax = "89.9", ClampMin = "-89.9", ClampMax = "89.9"))
	float ViewPitchMin;

	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "-89.9", UIMax = "89.9", ClampMin = "-89.9", ClampMax = "89.9"))
	float ViewPitchMax;

	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	float BlendTime;

	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	ECommonCameraModeBlendFunction BlendFunction;

	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	float BlendExponent;

	float BlendAlpha;

	float BlendWeight;

protected:
	UPROPERTY(transient)
	uint32 bResetInterpolation : 1;
};



/**
 * 카메라 모드 클래스 스택
 *
 *	
 */
UCLASS()
class UCommonCameraModeStack : public UObject
{
	GENERATED_BODY()

public:

	UCommonCameraModeStack();

	void ActivateStack();
	void DeactivateStack();

	bool IsStackActivate() const { return bIsActive; }

	void PushCameraMode(TSubclassOf<UCommonCameraMode> CameraModeClass);

	bool EvaluateStack(float DeltaTime, FCommonCameraModeView& OutCameraModeView);

	void GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const;

protected:

	UCommonCameraMode* GetCameraModeInstance(TSubclassOf<UCommonCameraMode> CameraModeClass);

	void UpdateStack(float DeltaTime);
	void BlendStack(FCommonCameraModeView& OutCameraModeView) const;

protected:

	bool bIsActive;

	UPROPERTY()
	TArray<TObjectPtr<UCommonCameraMode>> CameraModeInstances;

	UPROPERTY()
	TArray<TObjectPtr<UCommonCameraMode>> CameraModeStack;

};