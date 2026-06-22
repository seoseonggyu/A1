// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "Extension/UIExtensionTypes.h"
#include "UIExtensionSubsystem.generated.h"

class UUserWidget;

DECLARE_LOG_CATEGORY_EXTERN(UIExtensionSubsystemLog, Log, All);

/**
 * UI Extension 시스템 흐름
 *
 * [공급 측 - GameFeature]              [수신 측 - HUD]
 * RegisterExtensionAsWidget()          RegisterExtensionPoint()
 *         │                                     │
 *         ▼                                     ▼
 *   FUIExtension 생성                   FUIExtensionPoint 생성
 *   (내부 등록 정보)                    (내부 등록 정보)
 *         │                                     │
 *         └──────────── 매칭 ───────────────────┘
 *                         │
 *                         ▼
 *              FUIExtensionRequest 생성
 *                    (장착 요청)
 *                         │
 *                         ▼
 *              Point의 콜백 호출 (Added)
 *                         │
 *                         ▼
 *              실제 위젯 인스턴스 생성
 */

//-----------------------------------------------------------------------------
// FUIExtensionPoint
//-----------------------------------------------------------------------------

/**
 * 확장 포인트 내부 등록 정보 (수신 측 - 슬롯)
 *
 * 서브시스템이 내부적으로 관리하는 구조체입니다.
 * 하나의 Point에 여러 Extension이 연결될 수 있습니다.
 */
struct FUIExtensionPoint
{
public:
	/** 확장 포인트 태그 */
	FGameplayTag ExtensionPointTag;

	/** 매칭 규칙 */
	EUIExtensionPointMatch MatchType = EUIExtensionPointMatch::ExactMatch;

	/** 허용되는 데이터 클래스 목록 */
	TArray<UClass*> AllowedDataClasses;

	/** 확장 추가/제거 시 호출되는 콜백 */
	FExtendExtensionPointDelegate Callback;

	/** 현재 연결된 Extension ID 목록 (Unregister 시 정리용) */
	TArray<int32> ConnectedExtensionIds;
};

//-----------------------------------------------------------------------------
// FUIExtension
//-----------------------------------------------------------------------------

/**
 * 확장 내부 등록 정보 (공급 측 - 콘텐츠)
 *
 * 서브시스템이 내부적으로 관리하는 구조체입니다.
 * PartialMatch 시 하나의 Extension이 여러 Point에 연결될 수 있습니다.
 */
struct FUIExtension
{
public:
	/** 대상 확장 포인트 태그 */
	FGameplayTag ExtensionPointTag;

	/** 우선순위 */
	int32 Priority = 0;

	/** 확장 데이터 (위젯 클래스 등) */
	TWeakObjectPtr<UObject> Data;

	/** 컨텍스트 오브젝트 (플레이어 등) */
	TWeakObjectPtr<UObject> ContextObject;

	/** 현재 연결된 Point ID 목록 (Unregister 시 정리용) */
	TArray<int32> ConnectedPointIds;
};

//-----------------------------------------------------------------------------
// UUIExtensionSubsystem
//-----------------------------------------------------------------------------

/**
 * UIExtension 관리 서브시스템
 *
 * GameFeature에서 동적으로 UI를 추가할 수 있는 태그 기반 확장 시스템을 제공합니다.
 * 확장 포인트(수신 측)와 확장(공급 측)을 매칭하여 연결합니다.
 */
UCLASS()
class COMMONUIEXTENSION_API UUIExtensionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UUIExtensionSubsystem();

	//-----------------------------------------------------------------------------
	// USubsystem 오버라이드
	//-----------------------------------------------------------------------------

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//-----------------------------------------------------------------------------
	// 확장 포인트 관리 (수신 측)
	//-----------------------------------------------------------------------------

	/**
	 * 확장 포인트를 등록합니다
	 * @param ExtensionPointTag 확장 포인트 식별 태그
	 * @param MatchType 매칭 규칙
	 * @param AllowedDataClasses 허용되는 데이터 클래스 목록 (빈 배열이면 모두 허용)
	 * @param Callback 확장 추가/제거 시 호출되는 콜백
	 * @return 등록 해제에 사용할 핸들
	 */
	FUIExtensionPointHandle RegisterExtensionPoint(FGameplayTag ExtensionPointTag, EUIExtensionPointMatch MatchType, const TArray<UClass*>& AllowedDataClasses, const FExtendExtensionPointDelegate& Callback);

	/**
	 * 확장 포인트 등록을 해제합니다
	 * @param Handle 등록 시 받은 핸들
	 */
	void UnregisterExtensionPoint(FUIExtensionPointHandle Handle);

	//-----------------------------------------------------------------------------
	// 확장 관리 (공급 측)
	//-----------------------------------------------------------------------------

	/**
	 * 위젯 클래스를 확장으로 등록합니다
	 * @param ExtensionPointTag 대상 확장 포인트 태그
	 * @param WidgetClass 위젯 클래스
	 * @param Priority 우선순위 (높을수록 먼저 처리)
	 * @return 등록 해제에 사용할 핸들
	 */
	UFUNCTION(BlueprintCallable, Category = "UI Extension", meta = (AutoCreateRefTerm = "ExtensionPointTag"))
	FUIExtensionHandle RegisterExtensionAsWidget(UPARAM(meta = (Categories = "UI.ExtensionPoint")) FGameplayTag ExtensionPointTag, TSubclassOf<UUserWidget> WidgetClass, int32 Priority = 0);

	/**
	 * 데이터를 확장으로 등록합니다
	 * @param ExtensionPointTag 대상 확장 포인트 태그
	 * @param ContextObject 컨텍스트 오브젝트
	 * @param Data 데이터 오브젝트
	 * @param Priority 우선순위
	 * @return 등록 해제에 사용할 핸들
	 */
	UFUNCTION(BlueprintCallable, Category = "UI Extension", meta = (AutoCreateRefTerm = "ExtensionPointTag"))
	FUIExtensionHandle RegisterExtensionAsData(UPARAM(meta = (Categories = "UI.ExtensionPoint")) FGameplayTag ExtensionPointTag, UObject* ContextObject, UObject* Data, int32 Priority = 0);

	/**
	 * 확장 등록을 해제합니다
	 * @param Handle 등록 시 받은 핸들
	 */
	UFUNCTION(BlueprintCallable, Category = "UI Extension")
	void UnregisterExtension(FUIExtensionHandle Handle);

protected:
	/** 확장 포인트와 확장을 매칭합니다 */
	void MatchExtensionToPoint(int32 ExtensionId, FUIExtension& Extension);

	/** 확장과 확장 포인트를 매칭합니다 */
	void MatchPointToExtension(int32 PointId, FUIExtensionPoint& Point);

	/** 태그가 매칭되는지 확인합니다 */
	bool DoesTagMatch(FGameplayTag ExtensionTag, FGameplayTag PointTag, EUIExtensionPointMatch MatchType) const;

	/** 데이터 클래스가 허용되는지 확인합니다 */
	bool IsDataClassAllowed(const UClass* DataClass, const TArray<UClass*>& AllowedClasses) const;

	/** 확장 요청을 생성합니다 */
	FUIExtensionRequest CreateExtensionRequest(int32 ExtensionId, const FUIExtension& Extension) const;

	/** 다음 확장 포인트 ID를 반환합니다 */
	int32 GetNextExtensionPointId() { return NextExtensionPointId++; }

	/** 다음 확장 ID를 반환합니다 */
	int32 GetNextExtensionId() { return NextExtensionId++; }

protected:
	/** 등록된 확장 포인트 맵 */
	TMap<int32, FUIExtensionPoint> ExtensionPoints;

	/** 등록된 확장 맵 */
	TMap<int32, FUIExtension> Extensions;

	/** 다음 확장 포인트 ID */
	int32 NextExtensionPointId = 1;

	/** 다음 확장 ID */
	int32 NextExtensionId = 1;
};
