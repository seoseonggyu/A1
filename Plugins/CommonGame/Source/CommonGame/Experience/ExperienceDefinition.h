// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ExperienceDefinition.generated.h"

/**
 * Experience 정의 데이터 에셋
 *
 * 게임 모드에서 사용할 GameFeature 플러그인 조합을 정의합니다.
 * 서버가 Experience를 선택하면 모든 클라이언트가 동일한 Feature를 로드합니다.
 */
UCLASS(BlueprintType, Const)
class COMMONGAME_API UExperienceDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UExperienceDefinition();

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

public:
	/** 기본 Pawn 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
	TSubclassOf<APawn> DefaultPawnClass;

	/**
	 * 활성화할 GameFeature 목록
	 * 에디터에서 GameFeatureData 에셋을 드롭다운으로 선택 가능
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Features", meta = (AllowedTypes = "GameFeatureData"))
	TArray<FPrimaryAssetId> GameFeaturesToEnable;
};