// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameStateComponent.h"
#include "Coro.h"
#include "ExperienceManagerComponent.generated.h"

class UExperienceDefinition;

DECLARE_LOG_CATEGORY_EXTERN(ExperienceManagerLog, Log, All);

/** Experience 로드 상태 */
UENUM(BlueprintType)
enum class EExperienceLoadState : uint8
{
	Unloaded,
	Loading,
	LoadingGameFeatures,
	Loaded,
	Deactivating
};

/** Experience 로드 완료 델리게이트 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnExperienceLoaded, const UExperienceDefinition*);

/**
 * Experience 로딩 및 동기화를 관리하는 GameState 컴포넌트
 *
 * GameState에 추가되어 Experience의 전체 생명주기를 관리합니다.
 * 서버에서 Experience를 설정하면 클라이언트에 자동으로 복제됩니다.
 *
 * 동작 방식:
 * - 서버: SetCurrentExperience() 호출 → Experience 로드 → GameFeature 로드
 * - 클라이언트: OnRep에서 ExperienceId 수신 → 동일하게 로드
 * - 양쪽: 로드 완료 시 OnExperienceLoaded 브로드캐스트
 */
UCLASS()
class COMMONGAME_API UExperienceManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	UExperienceManagerComponent(const FObjectInitializer& ObjectInitializer);


	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	* 현재 Experience 설정 (서버 전용)
	* @param ExperienceId 로드할 Experience의 PrimaryAssetId
	*/
	void SetCurrentExperienceAuth(FPrimaryAssetId ExperienceId);

	/** 현재 Experience가 로드되었는지 확인 */
	bool IsExperienceLoaded() const { return LoadState == EExperienceLoadState::Loaded; }

	/** 현재 로드 상태 반환 */
	EExperienceLoadState GetLoadState() const { return LoadState; }

	/** 현재 로드된 Experience 반환 (로드 완료 시에만 유효) */
	const UExperienceDefinition* GetCurrentExperienceChecked() const;

	//-----------------------------------------------------------------------------
	// Experience 대기 코루틴 (Static)
	//-----------------------------------------------------------------------------

	/** Experience 로드 완료까지 대기합니다 (높은 우선순위) */
	static TCoroTask<const UExperienceDefinition*> WaitForExperienceLoaded_HighStaticCoroutine(UObject* WorldContextObject);

	/** Experience 로드 완료까지 대기합니다 (일반 우선순위) */
	static TCoroTask<const UExperienceDefinition*> WaitForExperienceLoadedStaticCoroutine(UObject* WorldContextObject);

	/** Experience 로드 완료까지 대기합니다 (낮은 우선순위) */
	static TCoroTask<const UExperienceDefinition*> WaitForExperienceLoaded_LowStaticCoroutine(UObject* WorldContextObject);


protected:

	/** 복제된 ExperienceId 수신 시 호출 */
	UFUNCTION()
	void OnRep_CurrentExperienceId();

private:
	/** Experience 로드 완료 대기 내부 구현 */
	static TCoroTask<const UExperienceDefinition*> WaitForExperienceLoadedInternalCoroutine(UObject* WorldContextObject, int32 Priority);


	/** Experience 로드 코루틴 */
	TCoroTask<void> LoadExperienceCoroutine();

	/** GameFeature 플러그인 로드 코루틴 */
	TCoroTask<void> LoadGameFeatureCoroutine(FString PluginURL) const;


private:
	/** 현재 Experience ID (복제됨) */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentExperienceId)
	FPrimaryAssetId CurrentExperienceId;

	/** 로드된 Experience 에셋 */
	UPROPERTY()
	TObjectPtr<const UExperienceDefinition> CurrentExperience;

	/** 현재 로드 상태 */
	EExperienceLoadState LoadState = EExperienceLoadState::Unloaded;

	/** 활성화된 GameFeature 플러그인 URL 목록 */
	TArray<FString> GameFeaturePluginURLs;

};