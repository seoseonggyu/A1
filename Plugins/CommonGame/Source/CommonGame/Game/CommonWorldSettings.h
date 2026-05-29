// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "CommonWorldSettings.generated.h"

class UExperienceDefinition;

/**
 * Experience 시스템을 위한 WorldSettings
 *
 * 맵별로 기본 Experience를 설정할 수 있습니다.
 * 에디터에서 World Settings 패널을 통해 Experience를 선택합니다.
 */
UCLASS()
class COMMONGAME_API ACommonWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:
	ACommonWorldSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** 이 맵에서 사용할 기본 Experience */
	FPrimaryAssetId GetDefaultExperienceId() const { return DefaultExperienceId; }

public:
	/** 이 맵에서 사용할 기본 Experience */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Experience", meta = (AllowedTypes = "ExperienceDefinition"))
	FPrimaryAssetId DefaultExperienceId;
};
