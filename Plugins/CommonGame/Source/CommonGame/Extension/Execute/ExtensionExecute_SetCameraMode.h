// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Extension/Execute/ExtensionExecute.h"
#include "ExtensionExecute_SetCameraMode.generated.h"

class UCommonCameraMode;

/**
 * 카메라 모드 설정을 담당하는 Execute
 *
 * GameplayCameraComponent에 카메라 모드를 설정합니다.
 * Pawn이 아닌 Actor에 적용하면 동작하지 않습니다.
 */
USTRUCT(BlueprintType, meta = (DisplayName = "Set Camera Mode"))
struct COMMONGAME_API FExtensionExecute_SetCameraMode : public FExtensionExecute
{
	GENERATED_BODY()

public:
	virtual void OnActivate(AActor* Owner) const override;
	virtual void OnDeactivate(AActor* Owner) const override;


public:
	/** 카메라 모드 클래스 (클라이언트 전용) */
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (AssetBundles = "Client"))
	TSubclassOf<UCommonCameraMode> CameraModeClass;

private:
	/** 카메라 모드 활성화 여부 (런타임 상태) */
	mutable bool bIsActive = false;
};
