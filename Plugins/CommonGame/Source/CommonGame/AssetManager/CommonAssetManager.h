// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "CommonAssetManager.generated.h"

class UCommonPrimaryDataAsset;

DECLARE_LOG_CATEGORY_EXTERN(CommonAssetManagerLog, Log, All);

/**
 * CommonGame용 커스텀 에셋 매니저
 *
 * UCommonPrimaryDataAsset의 번들 로딩 상태 추적 및 헬퍼 함수를 제공합니다.
 * 비동기 번들 로딩은 코루틴을 통해 제공됩니다: co_await Coro::LoadBundle(Asset)
 */
UCLASS()
class COMMONGAME_API UCommonAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	//-----------------------------------------------------------------------------
	// UAssetManager 오버라이드
	//-----------------------------------------------------------------------------

	virtual TSharedPtr<FStreamableHandle> ChangeBundleStateForPrimaryAssets(const TArray<FPrimaryAssetId>& AssetsToChange, const TArray<FName>& AddBundles, const TArray<FName>& RemoveBundles, bool bRemoveAllBundles, FAssetManagerLoadParams&& LoadParams, UE::FSourceLocation Location = UE::FSourceLocation::Current()) override;

public:
	/** CommonAssetManager 싱글톤 반환합니다 */
	static UCommonAssetManager& Get();

	//-----------------------------------------------------------------------------
	// 번들 로딩 API
	//-----------------------------------------------------------------------------

	/**
	 * 에셋의 번들이 로딩 완료되었는지 확인합니다
	 * @param Asset 확인할 PrimaryDataAsset
	 * @return 로딩 완료 여부 (로딩 요청이 없었으면 false)
	 */
	bool IsBundleLoaded(const UCommonPrimaryDataAsset* Asset) const;

	/**
	 * 에셋의 번들을 동기 로딩합니다
	 * 이미 로딩 완료된 경우 즉시 반환합니다.
	 * 번들 이름은 현재 NetMode에 따라 자동 결정됩니다 (Client/Server).
	 * @param Asset 로딩할 PrimaryDataAsset
	 */
	void LoadBundle(const UCommonPrimaryDataAsset* Asset);

private:
	/** 로딩 완료된 에셋 목록 */
	TSet<FPrimaryAssetId> LoadedAssets;
};