// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Action/GameFeatureAction_WorldNetworkBase.h"
#include "GameplayTagContainer.h"
#include "Extension/UIExtensionTypes.h"
#include "GameFeatureAction_AddWidgets.generated.h"

class UCommonActivatableWidget;
class UUserWidget;

/**
 * 레이아웃 추가 요청
 *
 * PrimaryGameLayout의 특정 레이어에 레이아웃 위젯을 추가합니다.
 */
USTRUCT()
struct FLayoutRequest
{
	GENERATED_BODY()

public:
	/** 스폰할 레이아웃 위젯 클래스 */
	UPROPERTY(EditAnywhere, Category = "UI", meta = (AssetBundles = "Client"))
	TSoftClassPtr<UCommonActivatableWidget> LayoutClass;

	/** 삽입할 레이어 태그 (예: UI.Layer.Game) */
	UPROPERTY(EditAnywhere, Category = "UI", meta = (Categories = "UI.Layer"))
	FGameplayTag LayerTag;
};

/**
 * UIExtensionPoint 위젯 추가 요청
 *
 * UIExtension 시스템을 통해 특정 ExtensionPoint에 위젯을 추가합니다.
 */
USTRUCT()
struct FUIExtensionPointRequest
{
	GENERATED_BODY()

public:
	/** 스폰할 위젯 클래스 */
	UPROPERTY(EditAnywhere, Category = "UI", meta = (AssetBundles = "Client"))
	TSoftClassPtr<UUserWidget> WidgetClass;

	/** 삽입할 ExtensionPoint 태그 (예: UI.ExtensionPoint.Crosshair) */
	UPROPERTY(EditAnywhere, Category = "UI", meta = (Categories = "UI.ExtensionPoint"))
	FGameplayTag ExtensionPointTag;
};

/**
 * UI 위젯을 추가하는 GameFeatureAction
 *
 * 두 가지 방식으로 UI를 추가할 수 있습니다:
 * 1. Layouts: PrimaryGameLayout의 레이어에 HUD 프레임 추가
 * 2. ExtensionPoints: UIExtension 시스템으로 슬롯에 개별 위젯 삽입
 */
UCLASS(meta = (DisplayName = "Add Widgets"))
class COMMONUIEXTENSION_API UGameFeatureAction_AddWidgets : public UGameFeatureAction_WorldNetworkBase
{
	GENERATED_BODY()

public:
	UGameFeatureAction_AddWidgets();

	//-----------------------------------------------------------------------------
	// UGameFeatureAction_WorldNetworkBase 오버라이드
	//-----------------------------------------------------------------------------

	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

private:
	/** HUD 컴포넌트 확장 이벤트 핸들러 */
	void HandleActorExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext);

	/** 위젯 추가 */
	void AddWidgets(AActor* Actor, FGameFeatureStateChangeContext ChangeContext);

	/** 위젯 제거 */
	void RemoveWidgets(AActor* Actor, FGameFeatureStateChangeContext ChangeContext);

	/** 모든 위젯 제거 */
	void Reset(const FGameFeatureStateChangeContext& ChangeContext);

public:
	/** PrimaryGameLayout의 레이어에 추가할 레이아웃 */
	UPROPERTY(EditAnywhere, Category = "UI|Layout", meta = (TitleProperty = "{LayerTag} -> {LayoutClass}"))
	TArray<FLayoutRequest> Layouts;

	/** UIExtensionPoint에 추가할 위젯 */
	UPROPERTY(EditAnywhere, Category = "UI|ExtensionPoint", meta = (TitleProperty = "{ExtensionPointTag} -> {WidgetClass}"))
	TArray<FUIExtensionPointRequest> ExtensionPoints;

private:
	/** Actor별 데이터 */
	struct FPerActorData
	{
		TArray<TWeakObjectPtr<UCommonActivatableWidget>> LayoutsAdded;
		TArray<FUIExtensionHandle> ExtensionHandles;
	};

	/** Context별 데이터 */
	struct FPerContextData
	{
		TArray<TSharedPtr<struct FComponentRequestHandle>> ComponentRequests;
		TMap<FObjectKey, FPerActorData> ActorData;
	};

	/** GameFeature Context별 데이터 맵 */
	TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;
};