// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameModeBase.h"
#include "Coro.h"
#include "CommonGameModeBase.generated.h"

class UExperienceDefinition;

DECLARE_LOG_CATEGORY_EXTERN(CommonGameModeLog, Log, All);

/**
 * Experience 시스템을 지원하는 GameMode 베이스 클래스
 *
 * Experience 로딩이 완료될 때까지 Pawn 스폰을 대기하고,
 * Experience에 정의된 PawnClass를 사용하여 스폰합니다.
 */
UCLASS()
class COMMONGAME_API ACommonGameModeBase : public AModularGameModeBase
{
	GENERATED_BODY()

public:
	ACommonGameModeBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;


protected:
	/** ExperienceManager를 통해 Experience 로드 완료 여부 확인 */
	bool IsExperienceLoaded() const;

	/** Experience 로드 완료 시 호출 */
	virtual void OnExperienceLoaded(const UExperienceDefinition* Experience);

private:
	/** WorldSettings에서 Experience를 가져와 로드 시작 */
	TCoroTask<void> StartExperienceLoadCoroutine();

};