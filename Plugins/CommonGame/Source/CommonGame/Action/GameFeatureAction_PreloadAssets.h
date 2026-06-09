// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Action/GameFeatureAction_WorldNetworkBase.h"
#include "GameFeatureAction_PreloadAssets.generated.h"

class UCommonPrimaryDataAsset;

//-----------------------------------------------------------------------------
// FPreloadAssetPath
//-----------------------------------------------------------------------------

/**
 * 프리로드할 에셋 폴더 경로 설정
 */
USTRUCT(BlueprintType)
struct COMMONGAME_API FPreloadAssetPath
{
	GENERATED_BODY()

public:
	/** 에셋 폴더 경로 */
	UPROPERTY(EditAnywhere, meta = (ContentDir, LongPackageName))
	FDirectoryPath Path;

	/** 하위 폴더까지 포함 */
	UPROPERTY(EditAnywhere)
	bool bRecursive = true;
};

//-----------------------------------------------------------------------------
// UGameFeatureAction_PreloadAssets
//-----------------------------------------------------------------------------

/**
 * CommonPrimaryDataAsset을 프리로드하는 GameFeatureAction
 *
 * Experience 활성화 시 지정된 에셋들의 번들을 미리 로드합니다.
 * 개별 에셋 선택 또는 폴더 경로 지정이 가능합니다.
 */
UCLASS(DisplayName = "Preload Assets")
class COMMONGAME_API UGameFeatureAction_PreloadAssets : public UGameFeatureAction_WorldNetworkBase
{
	GENERATED_BODY()

public:
	UGameFeatureAction_PreloadAssets();

	//-----------------------------------------------------------------------------
	// UGameFeatureAction_WorldNetworkBase 오버라이드
	//-----------------------------------------------------------------------------

	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;

#if WITH_EDITORONLY_DATA
	virtual void AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData) override;
#endif

public:
	/** 프리로드할 개별 에셋 목록 */
	UPROPERTY(EditAnywhere, Category = "Assets")
	TArray<TObjectPtr<UCommonPrimaryDataAsset>> Assets;

	/** 프리로드할 에셋 폴더 경로 */
	UPROPERTY(EditAnywhere, Category = "Assets")
	TArray<FPreloadAssetPath> AssetPaths;

	/** 폴더 검색 시 무시할 클래스 목록 */
	UPROPERTY(EditAnywhere, Category = "Assets")
	TArray<TSubclassOf<UCommonPrimaryDataAsset>> IgnoreClasses;
};
