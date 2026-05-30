// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameStateBase.h"
#include "CommonGameStateBase.generated.h"

class UExperienceManagerComponent;

/**
 * Experience 시스템을 지원하는 GameState 베이스 클래스
 *
 * ExperienceManagerComponent를 기본으로 포함하여
 * Experience 로딩 및 GameFeature 관리를 담당합니다.
 */
UCLASS()
class COMMONGAME_API ACommonGameStateBase : public AModularGameStateBase
{
	GENERATED_BODY()

public:
	ACommonGameStateBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


	/** Experience Manager 컴포넌트 반환 */
	UExperienceManagerComponent* GetExperienceManagerComponent() const { return ExperienceManagerComponent; }

private:
	/** Experience 관리 컴포넌트 */
	UPROPERTY()
	TObjectPtr<UExperienceManagerComponent> ExperienceManagerComponent;
};
