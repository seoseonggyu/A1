// Copyright Epic Games, Inc. All Rights Reserved.

#include "Action/GameFeatureAction_WorldNetworkBase.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_WorldNetworkBase)

#if WITH_EDITORONLY_DATA
#include "Engine/AssetManager.h"
#include "GameFeaturesSubsystemSettings.h"
#include "Utility/SoftPathCollector.h"
#endif

UGameFeatureAction_WorldNetworkBase::UGameFeatureAction_WorldNetworkBase() : bClientAction(false), bServerAction(false)
{
}

void UGameFeatureAction_WorldNetworkBase::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	FGameFeatureStateChangeContext ChangeContext(Context);

	// PIE 멀티플레이어에서 GameFeature 활성화 이후 생성되는 GameInstance/World 감지
	// - OnStartGameInstance: 새 GameInstance 생성 시 (PIE 클라이언트 추가 등)
	// - OnGameInstanceWorldChanged: 기존 GameInstance의 World 전환 시 (Pending → 실제 World)
	FDelegateHandle StartHandle = FWorldDelegates::OnStartGameInstance.AddUObject(this, &ThisClass::HandleGameInstanceStart, ChangeContext);
	GameInstanceStartHandles.Add(ChangeContext, StartHandle);

	FDelegateHandle WorldChangedHandle = FWorldDelegates::OnGameInstanceWorldChanged.AddUObject(this, &ThisClass::HandleGameInstanceWorldChanged, ChangeContext);
	GameInstanceWorldChangedHandles.Add(ChangeContext, WorldChangedHandle);

	// 현재 존재하는 모든 월드에 대해 AddToWorld 호출
	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		if (Context.ShouldApplyToWorldContext(WorldContext))
		{
			UWorld* World = WorldContext.World();
			if (World && World->IsGameWorld() && ShouldApplyToNetMode(World->GetNetMode()))
			{
				AddToWorld(WorldContext, ChangeContext);
			}
		}
	}
}

void UGameFeatureAction_WorldNetworkBase::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	FGameFeatureStateChangeContext ChangeContext(Context);

	// TODO: 여기서도 각 GameInstance마다?
	
	// OnStartGameInstance 해제
	if (FDelegateHandle* HandlePtr = GameInstanceStartHandles.Find(ChangeContext))
	{
		FWorldDelegates::OnStartGameInstance.Remove(*HandlePtr);
		GameInstanceStartHandles.Remove(ChangeContext);
	}

	// OnGameInstanceWorldChanged 해제
	if (FDelegateHandle* HandlePtr = GameInstanceWorldChangedHandles.Find(ChangeContext))
	{
		FWorldDelegates::OnGameInstanceWorldChanged.Remove(*HandlePtr);
		GameInstanceWorldChangedHandles.Remove(ChangeContext);
	}
}

void UGameFeatureAction_WorldNetworkBase::HandleGameInstanceStart(UGameInstance* GameInstance, FGameFeatureStateChangeContext ChangeContext)
{
	if (const FWorldContext* WorldContext = GameInstance->GetWorldContext())
	{
		UWorld* World = WorldContext->World();
		if (World && World->IsGameWorld() && ChangeContext.ShouldApplyToWorldContext(*WorldContext) && ShouldApplyToNetMode(World->GetNetMode()))
		{
			AddToWorld(*WorldContext, ChangeContext);
		}
	}
}

void UGameFeatureAction_WorldNetworkBase::HandleGameInstanceWorldChanged(UGameInstance* GameInstance, UWorld* OldWorld, UWorld* NewWorld, FGameFeatureStateChangeContext ChangeContext)
{
	if (!NewWorld || !NewWorld->IsGameWorld())
	{
		return;
	}

	const FWorldContext* WorldContext = GameInstance->GetWorldContext();
	if (!WorldContext)
	{
		return;
	}

	if (ChangeContext.ShouldApplyToWorldContext(*WorldContext) && ShouldApplyToNetMode(NewWorld->GetNetMode()))
	{
		AddToWorld(*WorldContext, ChangeContext);
	}
}

bool UGameFeatureAction_WorldNetworkBase::ShouldApplyToNetMode(ENetMode NetMode) const
{
	switch (NetMode)
	{
	case NM_Client:
		return bClientAction;
	case NM_DedicatedServer:
		return bServerAction;
	case NM_ListenServer:
	case NM_Standalone:
		return bClientAction || bServerAction;
	default:
		return false;
	}
}

#if WITH_EDITORONLY_DATA
void UGameFeatureAction_WorldNetworkBase::AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData)
{
	if (!UAssetManager::IsInitialized())
	{
		return;
	}

	// 리플렉션으로 Client/Server별 SoftPtr 경로 수집
	TArray<FSoftObjectPath> ClientPaths;
	TArray<FSoftObjectPath> ServerPaths;
	FSoftPathCollector::CollectSoftObjectPaths(GetClass(), this, ClientPaths, ServerPaths);

	// 메타데이터에 명시된 대로 번들에 등록
	for (const FSoftObjectPath& Path : ClientPaths)
	{
		AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateClient, Path.GetAssetPath());
	}
	for (const FSoftObjectPath& Path : ServerPaths)
	{
		AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateServer, Path.GetAssetPath());
	}
}
#endif
