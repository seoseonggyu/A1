// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/Awaiter.h"
#include "AssetManager/CommonAssetManager.h"
#include "DataAsset/CommonPrimaryDataAsset.h"
#include "GameFeaturesSubsystem.h"
#include "GameFeaturesProjectPolicies.h"

namespace Coro::Private
{

//-----------------------------------------------------------------------------
// TLoadCommonDataAssetAwaiter
//-----------------------------------------------------------------------------

/**
 * UCommonPrimaryDataAsset의 번들을 로드하는 Awaiter
 *
 * CommonAssetManager를 사용하여 번들을 로드합니다.
 * 번들 이름은 현재 NetMode에 따라 자동 결정됩니다 (Client/Server).
 */
template<typename T>
class TLoadCommonDataAssetAwaiter : public TAsyncAwaiterBase<TLoadCommonDataAssetAwaiter<T>>
{
	using Super = TAsyncAwaiterBase<TLoadCommonDataAssetAwaiter<T>>;

public:
	TLoadCommonDataAssetAwaiter(UObject* InOwner, const UCommonPrimaryDataAsset* InAsset)
		: Super(InOwner)
		, Asset(InAsset)
	{
	}

	//-----------------------------------------------------------------------------
	// TAsyncAwaiterBase 메서드
	//-----------------------------------------------------------------------------

	/** 이미 번들이 로드되어 있으면 suspend하지 않습니다 */
	bool Ready() const
	{
		if (!Asset)
		{
			return true;
		}

		return UCommonAssetManager::Get().IsBundleLoaded(Asset);
	}

	/** 번들 비동기 로드를 시작합니다 */
	void Suspend()
	{
		UCommonAssetManager& AssetManager = UCommonAssetManager::Get();
		const FPrimaryAssetId AssetId = Asset->GetPrimaryAssetId();
		const TArray<FName> BundleNames = UGameFeaturesSubsystem::Get().GetPolicy().GetPreloadBundleStateForGameFeature();

		FAssetManagerLoadParams LoadParams;
		LoadParams.Priority = FStreamableManager::AsyncLoadHighPriority;
		LoadParams.OnComplete = FStreamableDelegateWithHandle::CreateLambda(
			[CapturedOwner = this->Owner, CapturedContext = this->Context](TSharedPtr<FStreamableHandle>)
			{
				SafeResume(CapturedOwner, CapturedContext);
			}
		);

		StreamHandle = AssetManager.ChangeBundleStateForPrimaryAssets(
			{ AssetId },
			BundleNames,
			{},
			false,
			MoveTemp(LoadParams)
		);
	}

	/** Cast된 에셋을 반환합니다 */
	const T* GetResult() const
	{
		return Cast<T>(Asset);
	}

private:
	/** 로드할 에셋 */
	const UCommonPrimaryDataAsset* Asset;

	/** 스트리밍 핸들 */
	TSharedPtr<FStreamableHandle> StreamHandle;
};

} // namespace Coro::Private

namespace Coro::Async
{

//-----------------------------------------------------------------------------
// 사용자 함수
//-----------------------------------------------------------------------------

/**
 * UCommonPrimaryDataAsset의 번들을 로드합니다
 *
 * 번들 이름은 현재 NetMode에 따라 자동 결정됩니다 (Client/Server).
 * 이미 로드된 경우 즉시 반환합니다.
 *
 * @tparam T 반환할 에셋 타입 (UCommonPrimaryDataAsset 파생)
 * @param Owner 코루틴을 소유한 UObject (파괴 시 안전하게 정리됨)
 * @param Asset 로드할 CommonPrimaryDataAsset
 * @return Cast된 에셋 포인터
 *
 * @code
 * TCoroTask<void> LoadWeaponCoroutine(const UCommonPrimaryDataAsset* Asset)
 * {
 *     const UWeaponDefinition* Weapon = co_await Coro::Async::LoadCommonDataAsset<UWeaponDefinition>(this, Asset);
 *     // 번들 로드 완료 - 메시, 사운드 등 사용 가능
 * }
 * @endcode
 */
template<typename T>
Private::TLoadCommonDataAssetAwaiter<T> LoadCommonDataAsset(UObject* Owner, const UCommonPrimaryDataAsset* Asset)
{
	return Private::TLoadCommonDataAssetAwaiter<T>(Owner, Asset);
}

} // namespace Coro::Async