// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Extension/ActorExtension.h"
#include "Coro.h"
#include "ActorExtensionWorldSubsystem.generated.h"

class UExperienceDefinition;
struct FComponentRequestHandle;

DECLARE_LOG_CATEGORY_EXTERN(ActorExtensionWorldSubsystemLog, Log, All);

//-----------------------------------------------------------------------------
// FRegisterActorData
//-----------------------------------------------------------------------------

/** Register 상태 Actor 데이터 (Actor + Extension 복사본 묶음) */
struct FRegisterActorData
{
	TWeakObjectPtr<AActor> Actor;
	TArray<FActorExtension> Extensions;
};

//-----------------------------------------------------------------------------
// FCompleteActorData
//-----------------------------------------------------------------------------

/** Complete 상태 Actor 데이터 (Deactivate용) */
struct FCompleteActorData
{
	TArray<FActorExtension> Extensions;
};

//-----------------------------------------------------------------------------
// FExtensionEntry
//-----------------------------------------------------------------------------

/** Extension 등록 정보 (Extension + 네트워크 Role 설정) */
struct FExtensionEntry
{
	FActorExtension Extension;
	bool bAddToLocallyControlled = false;
	bool bAddToSimulatedProxy = false;
};

//-----------------------------------------------------------------------------
// FClassExtensionMapping
//-----------------------------------------------------------------------------

/** 클래스별 Extension 정의 캐시 */
struct FClassExtensionMapping
{
	TArray<FExtensionEntry> Entries;
};

//-----------------------------------------------------------------------------
// UActorExtensionWorldSubsystem
//-----------------------------------------------------------------------------

/**
 * ActorExtension을 중앙에서 관리하는 WorldSubsystem
 *
 * 모든 Actor의 Extension을 관리하고 조건 평가를 수행합니다.
 * Experience 로드 완료 후 Tick을 시작하여 조건을 평가합니다.
 */
UCLASS()
class COMMONGAME_API UActorExtensionWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	//-----------------------------------------------------------------------------
	// UTickableWorldSubsystem 오버라이드
	//-----------------------------------------------------------------------------

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;
	virtual bool IsTickable() const override;

	//-----------------------------------------------------------------------------
	// Extension 등록 API
	//-----------------------------------------------------------------------------

	/**
	 * 클래스에 Extension 정의를 등록합니다
	 * @param TargetClass 대상 Actor 클래스
	 * @param Extension 등록할 Extension 정의 (복사됨)
	 * @param bAddToLocallyControlled LocallyControlled에 적용할지 여부
	 * @param bAddToSimulatedProxy SimulatedProxy에 적용할지 여부
	 */
	void RegisterExtensionForClass(UClass* TargetClass, const FActorExtension& Extension, bool bAddToLocallyControlled, bool bAddToSimulatedProxy);

	/**
	 * 클래스에서 Extension 정의를 해제합니다 (TargetClass의 모든 Extension 제거)
	 * @param TargetClass 대상 Actor 클래스
	 */
	void UnregisterExtensionsForClass(UClass* TargetClass);

	//-----------------------------------------------------------------------------
	// 조회 API
	//-----------------------------------------------------------------------------

	/** 특정 클래스에 Extension이 매핑되어 있는지 확인합니다 */
	bool HasExtensionsForClass(const UClass* ActorClass) const;

	/** Actor가 Complete 상태인지 확인합니다 */
	bool IsActorComplete(const AActor* Actor) const;

private:
	//-----------------------------------------------------------------------------
	// 내부 메서드
	//-----------------------------------------------------------------------------

	/** Experience 로드를 대기하고 완료 시 상태를 변경합니다 */
	TCoroTask<void> WaitForExperienceLoadedCoroutine();

	/** GFCM에 Extension Handler 등록합니다 */
	void RegisterWithGameFrameworkComponentManager();

	/** Actor Extension 이벤트 핸들러 (GameActorReady, ReceiverRemoved) */
	void HandleActorExtensionEvent(AActor* Actor, FName EventName);

	/**
	 * Actor에 대한 Extension 복사본을 생성합니다
	 * @return Extension이 매핑되어 있으면 true, 없으면 false
	 */
	bool TryCreateExtensions(AActor* Actor, TArray<FActorExtension>& OutExtensions);

	/** Actor 제거 시 정리합니다 (상태에 따라 Deactivate 처리) */
	void RemoveActor(AActor* Actor);

private:
	//-----------------------------------------------------------------------------
	// 상태별 Actor 컨테이너
	//-----------------------------------------------------------------------------

	/** Extension 확인 전 Actor 목록 (Instance 없음) */
	TArray<TWeakObjectPtr<AActor>> UncheckedActors;

	/** 조건 평가 대기 중인 Actor 목록 (Actor + Extension 복사본 묶음) */
	TArray<FRegisterActorData> RegisterActors;

	/** 활성화 완료된 Actor 맵 (Deactivate용) */
	TMap<TWeakObjectPtr<AActor>, FCompleteActorData> CompleteActors;

	//-----------------------------------------------------------------------------
	// Extension 캐시
	//-----------------------------------------------------------------------------

	/** 클래스별 Extension 정의 캐시 */
	TMap<UClass*, FClassExtensionMapping> ClassExtensionCache;

	//-----------------------------------------------------------------------------
	// 시스템 상태
	//-----------------------------------------------------------------------------

	/** Experience 로드 완료 여부 */
	bool bExperienceLoaded = false;

	/** GFCM Extension 핸들 목록 (클래스별) */
	TArray<TSharedPtr<FComponentRequestHandle>> ExtensionHandles;
};

