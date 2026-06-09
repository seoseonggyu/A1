// Copyright Epic Games, Inc. All Rights Reserved.

#include "Action/GameFeatureAction_PreloadAssets.h"
#include "DataAsset/CommonPrimaryDataAsset.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_PreloadAssets)

#if WITH_EDITORONLY_DATA
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManager.h"
#endif

UGameFeatureAction_PreloadAssets::UGameFeatureAction_PreloadAssets()
{
	// 클라이언트와 서버 모두에서 프리로드
	bClientAction = true;
	bServerAction = true;
}

void UGameFeatureAction_PreloadAssets::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	// 번들 로드만 수행하므로 별도 처리 없음
}

#if WITH_EDITORONLY_DATA
void UGameFeatureAction_PreloadAssets::AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData)
{
	if (!UAssetManager::IsInitialized())
	{
		return;
	}

	// 에셋의 번들 데이터를 병합하는 람다
	auto MergeAssetBundleData = [&AssetBundleData](UCommonPrimaryDataAsset* Asset)
	{
		if (!Asset)
		{
			return;
		}

		// 에셋의 번들 데이터 업데이트
		Asset->UpdateAssetBundleData();

		// 에셋의 번들 데이터를 우리 번들에 병합
		const FAssetBundleData& SourceBundleData = Asset->GetAssetBundleData();
		for (const FAssetBundleEntry& Entry : SourceBundleData.Bundles)
		{
			for (const FTopLevelAssetPath& BundleAsset : Entry.AssetPaths)
			{
				AssetBundleData.AddBundleAsset(Entry.BundleName, BundleAsset);
			}
		}
	};

	// 1. Assets 배열에서 직접 지정된 에셋들 처리
	for (UCommonPrimaryDataAsset* Asset : Assets)
	{
		MergeAssetBundleData(Asset);
	}

	// 2. AssetPaths에서 CommonPrimaryDataAsset 검색 후 번들 병합
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

	for (const FPreloadAssetPath& PreloadPath : AssetPaths)
	{
		if (PreloadPath.Path.Path.IsEmpty())
		{
			continue;
		}

		FARFilter Filter;
		Filter.PackagePaths.Add(FName(*PreloadPath.Path.Path));
		Filter.bRecursivePaths = PreloadPath.bRecursive;
		Filter.ClassPaths.Add(UCommonPrimaryDataAsset::StaticClass()->GetClassPathName());
		Filter.bRecursiveClasses = true;

		TArray<FAssetData> FoundAssets;
		AssetRegistry.GetAssets(Filter, FoundAssets);

		for (const FAssetData& AssetData : FoundAssets)
		{
			if (UCommonPrimaryDataAsset* Asset = Cast<UCommonPrimaryDataAsset>(AssetData.GetAsset()))
			{
				// IgnoreClasses에 포함된 클래스는 스킵
				bool bIgnored = false;
				for (const TSubclassOf<UCommonPrimaryDataAsset>& IgnoreClass : IgnoreClasses)
				{
					if (IgnoreClass && Asset->IsA(IgnoreClass))
					{
						bIgnored = true;
						break;
					}
				}

				if (!bIgnored)
				{
					MergeAssetBundleData(Asset);
				}
			}
		}
	}
}
#endif
