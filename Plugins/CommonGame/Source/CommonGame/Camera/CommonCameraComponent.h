
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Camera/CameraComponent.h"
#include "GameFramework/Actor.h"
#include "GameplayAbilitySpecHandle.h"

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

	/** 카메라 모드 결정의 "기본" 레이어를 설정한다. Ability가 덮어쓴 게 없을 때 사용된다. */
	void SetBaseCameraMode(TSubclassOf<UCommonCameraMode> NewBaseCameraMode);

	/** Ability가 실행되는 동안 카메라 모드를 임시로 덮어쓴다(Base보다 우선). */
	void SetAbilityCameraMode(TSubclassOf<UCommonCameraMode> NewCameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle);

	/** OwningSpecHandle이 건 오버라이드를 해제한다. 다른 Ability가 이미 새로 덮어썼으면(핸들 불일치) 무시한다. */
	void ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle);

protected:

	virtual void OnRegister() override;
	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;

	virtual void UpdateCameraModes();

	/** AbilityCameraModeClass가 있으면 그것을, 없으면 BaseCameraModeClass를 반환한다. */
	TSubclassOf<UCommonCameraMode> DetermineCameraMode() const;

protected:

	UPROPERTY()
	TObjectPtr<UCommonCameraModeStack> CameraModeStack;

	float FieldOfViewOffset;

	/** Ability 오버라이드가 없을 때 사용할 기본 카메라 모드. FExtensionExecute_SetCameraMode 등이 설정한다. */
	UPROPERTY(Transient)
	TSubclassOf<UCommonCameraMode> BaseCameraModeClass;

	/** Ability가 SetCameraMode로 건 임시 오버라이드. */
	UPROPERTY(Transient)
	TSubclassOf<UCommonCameraMode> AbilityCameraModeClass;

	/** AbilityCameraModeClass를 건 Ability의 SpecHandle. Clear 시 소유권 확인용. */
	FGameplayAbilitySpecHandle AbilityCameraModeOwningHandle;
};