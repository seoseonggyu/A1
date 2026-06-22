// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"
#include "CommonExtensionUserWidget.generated.h"

/**
 * 기본 위젯 클래스
 *
 * CommonUI의 UCommonUserWidget을 상속합니다.
 */
UCLASS(Abstract, meta = (DisableNativeTick))
class COMMONUIEXTENSION_API UCommonExtensionUserWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UCommonExtensionUserWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};