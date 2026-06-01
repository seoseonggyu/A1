// Copyright Epic Games, Inc. All Rights Reserved.

#include "DataAsset/CommonPrimaryDataAsset.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonPrimaryDataAsset)

#if WITH_EDITORONLY_DATA
#include "Engine/AssetManager.h"
#include "Utility/SoftPathCollector.h"

void UCommonPrimaryDataAsset::UpdateAssetBundleData()
{
	if (!UAssetManager::IsInitialized())
	{
		return;
	}

	AssetBundleData.Reset();

	// 메타데이터에 따라 Client/Server 번들에 등록
	TArray<FSoftObjectPath> ClientPaths;
	TArray<FSoftObjectPath> ServerPaths;
	FSoftPathCollector::CollectSoftObjectPaths(GetClass(), this, ClientPaths, ServerPaths);

	for (const FSoftObjectPath& Path : ClientPaths)
	{
		AssetBundleData.AddBundleAsset(FName(TEXT("Client")), Path.GetAssetPath());
	}
	for (const FSoftObjectPath& Path : ServerPaths)
	{
		AssetBundleData.AddBundleAsset(FName(TEXT("Server")), Path.GetAssetPath());
	}
}
#endif
