// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetManager/CommonAssetManager.h"
#include "DataAsset/CommonPrimaryDataAsset.h"
#include "GameFeaturesSubsystem.h"
#include "GameFeaturesProjectPolicies.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonAssetManager)

DEFINE_LOG_CATEGORY(CommonAssetManagerLog);

TSharedPtr<FStreamableHandle> UCommonAssetManager::ChangeBundleStateForPrimaryAssets(
	const TArray<FPrimaryAssetId>& AssetsToChange,
	const TArray<FName>& AddBundles,
	const TArray<FName>& RemoveBundles,
	bool bRemoveAllBundles,
	FAssetManagerLoadParams&& LoadParams,
	UE::FSourceLocation Location)
{
	// RemoveBundles 처리
	if (bRemoveAllBundles || RemoveBundles.Num() > 0)
	{
		for (const FPrimaryAssetId& AssetId : AssetsToChange)
		{
			LoadedAssets.Remove(AssetId);
		}
	}

	// AddBundles가 있으면 로딩 완료 시 Set에 추가
	if (AddBundles.Num() > 0)
	{
		TArray<FPrimaryAssetId> AssetsToChangeCopy = AssetsToChange;
		FStreamableDelegateWithHandle OriginalDelegate = MoveTemp(LoadParams.OnComplete);

		LoadParams.OnComplete = FStreamableDelegateWithHandle::CreateLambda(
			[this, AssetsToChangeCopy, OriginalDelegate](TSharedPtr<FStreamableHandle>)
			{
				// 로딩 완료 → Set에 추가
				for (const FPrimaryAssetId& AssetId : AssetsToChangeCopy)
				{
					LoadedAssets.Add(AssetId);
				}

				// 원본 콜백 실행
				if (OriginalDelegate.IsBound())
				{
					OriginalDelegate.Execute(nullptr);
				}
			});
	}

	return Super::ChangeBundleStateForPrimaryAssets(
		AssetsToChange, AddBundles, RemoveBundles, bRemoveAllBundles, MoveTemp(LoadParams), Location);
}

UCommonAssetManager& UCommonAssetManager::Get()
{
	UAssetManager* Singleton = UAssetManager::GetIfInitialized();

	if (UCommonAssetManager* CommonAssetManager = Cast<UCommonAssetManager>(Singleton))
	{
		return *CommonAssetManager;
	}

	UE_LOG(CommonAssetManagerLog, Fatal, TEXT("AssetManager가 UCommonAssetManager가 아닙니다. DefaultEngine.ini 설정을 확인하세요."));
	return *NewObject<UCommonAssetManager>();
}

bool UCommonAssetManager::IsBundleLoaded(const UCommonPrimaryDataAsset* Asset) const
{
	if (!Asset)
	{
		return false;
	}

	return LoadedAssets.Contains(Asset->GetPrimaryAssetId());
}

void UCommonAssetManager::LoadBundle(const UCommonPrimaryDataAsset* Asset)
{
	if (!Asset)
	{
		UE_LOG(CommonAssetManagerLog, Warning, TEXT("LoadBundle: Asset이 nullptr입니다"));
		return;
	}

	if (IsBundleLoaded(Asset))
	{
		return;
	}

	const FPrimaryAssetId AssetId = Asset->GetPrimaryAssetId();
	const TArray<FName> BundleNames = UGameFeaturesSubsystem::Get().GetPolicy().GetPreloadBundleStateForGameFeature();

	UE_LOG(CommonAssetManagerLog, Log, TEXT("번들 동기 로딩: %s"), *AssetId.ToString());

	FAssetManagerLoadParams LoadParams;
	TSharedPtr<FStreamableHandle> Handle = ChangeBundleStateForPrimaryAssets(
		{ AssetId },
		BundleNames,
		{},
		false,
		MoveTemp(LoadParams)
	);

	if (Handle.IsValid())
	{
		Handle->WaitUntilComplete();
	}
}