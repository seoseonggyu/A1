// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonCameraMode.h"
#include "CommonCameraMode_TopDown.generated.h"

/**
 *
 */
UCLASS(Abstract, Blueprintable)
class COMMONGAME_API UCommonCameraMode_TopDown : public UCommonCameraMode
{
	GENERATED_BODY()

public:
	UCommonCameraMode_TopDown();

protected:

	//~ULyraCameraMode interface
	virtual void UpdateView(float DeltaTime) override;
	//~End of ULyraCameraMode interface


};
