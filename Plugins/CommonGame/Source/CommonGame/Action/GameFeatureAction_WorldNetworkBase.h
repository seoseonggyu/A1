// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFeatureAction.h"
#include "GameFeaturesSubsystem.h"
#include "GameFeatureAction_WorldNetworkBase.generated.h"

struct FWorldContext;
class UGameInstance;

/**
 * 월드 및 네트워크 기반 GameFeatureAction의 베이스 클래스
 *
 * GameFeature 활성화 시 현재 존재하는 모든 월드와 이후 생성되는 월드에 대해
 * AddToWorld를 호출합니다. bClientAction/bServerAction으로 클라이언트/서버 필터링을 지원합니다.
 *
 * Asset Bundle 자동 등록:
 * - 파생 클래스의 TSoftObjectPtr/TSoftClassPtr 프로퍼티를 자동으로 Asset Bundle에 등록
 * - UPROPERTY의 AssetBundles 메타데이터에 따라 Client/Server 번들에 등록
 * - 미지정 시 번들에 등록하지 않음
 * - 예시: UPROPERTY(meta = (AssetBundles = "Client"))
 *         UPROPERTY(meta = (AssetBundles = "Server"))
 *         UPROPERTY(meta = (AssetBundles = "All"))
 */
UCLASS(Abstract)
class COMMONGAME_API UGameFeatureAction_WorldNetworkBase : public UGameFeatureAction
{
	GENERATED_BODY()
public:
	UGameFeatureAction_WorldNetworkBase();

	//-----------------------------------------------------------------------------
	// UGameFeatureAction 오버라이드
	//-----------------------------------------------------------------------------

	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
#if WITH_EDITORONLY_DATA
	virtual void AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData) override;
#endif

protected:
	/**
	 * 월드에 이 Action의 기능을 추가
	 * 파생 클래스에서 반드시 구현해야 합니다.
	 * @param WorldContext 대상 월드 컨텍스트
	 * @param ChangeContext GameFeature 상태 변경 컨텍스트
	 */
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) PURE_VIRTUAL(UGameFeatureAction_WorldNetworkBase::AddToWorld, );

	/**
	 * 현재 NetMode에서 이 Action을 실행해야 하는지 확인
	 * @param NetMode 확인할 네트워크 모드
	 * @return 실행해야 하면 true
	 */
	bool ShouldApplyToNetMode(ENetMode NetMode) const;

private:
	/** 새 GameInstance 시작 시 호출되는 핸들러 */
	void HandleGameInstanceStart(UGameInstance* GameInstance, FGameFeatureStateChangeContext ChangeContext);

	/** GameInstance의 World 변경 시 호출되는 핸들러 */
	void HandleGameInstanceWorldChanged(UGameInstance* GameInstance, UWorld* OldWorld, UWorld* NewWorld, FGameFeatureStateChangeContext ChangeContext);

public:
	/** 클라이언트에서 이 Action을 실행할지 */
	UPROPERTY(EditAnywhere, Category = "Network")
	uint8 bClientAction : 1;

	/** 서버에서 이 Action을 실행할지 */
	UPROPERTY(EditAnywhere, Category = "Network")
	uint8 bServerAction : 1;

private:
	/** OnStartGameInstance 델리게이트 핸들 */
	TMap<FGameFeatureStateChangeContext, FDelegateHandle> GameInstanceStartHandles;

	/** OnGameInstanceWorldChanged 델리게이트 핸들 */
	TMap<FGameFeatureStateChangeContext, FDelegateHandle> GameInstanceWorldChangedHandles;
};
