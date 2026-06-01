// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CommonPrimaryDataAsset.generated.h"

/**
 * Soft Reference 자동 프리로드를 지원하는 PrimaryDataAsset 베이스 클래스
 *
 * 내부의 모든 TSoftObjectPtr/TSoftClassPtr를 리플렉션으로 스캔하여
 * Asset Bundle에 자동 등록합니다. TInstancedStruct, TArray, TMap 내부도 지원합니다.
 *
 * 동작 방식:
 * - 에디터 저장 시 UpdateAssetBundleData가 호출됨
 * - FSoftPathCollector로 AssetBundles 메타데이터에 따라 Client/Server 번들에 등록
 * - 미지정 시 번들에 등록하지 않음
 *
 * 예시: UPROPERTY(meta = (AssetBundles = "Client"))
 *       UPROPERTY(meta = (AssetBundles = "Server"))
 *       UPROPERTY(meta = (AssetBundles = "All"))
 */
UCLASS(Abstract)
class COMMONGAME_API UCommonPrimaryDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITORONLY_DATA
	//-----------------------------------------------------------------------------
	// UPrimaryDataAsset 오버라이드
	//-----------------------------------------------------------------------------

	virtual void UpdateAssetBundleData() override;

	/** 번들 데이터 반환 */
	const FAssetBundleData& GetAssetBundleData() const { return AssetBundleData; }
#endif
};
