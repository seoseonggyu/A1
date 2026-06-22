// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/DynamicEntryBoxBase.h"
#include "GameplayTagContainer.h"
#include "Extension/UIExtensionTypes.h"
#include "UIExtensionPointWidget.generated.h"

class UUIExtensionSubsystem;
class UUserWidget;

DECLARE_LOG_CATEGORY_EXTERN(UIExtensionPointWidgetLog, Log, All);

/** 위젯 클래스 결정 델리게이트 */
DECLARE_DELEGATE_RetVal_OneParam(TSubclassOf<UUserWidget>, FOnGetWidgetClassForData, const FUIExtensionRequest& /*Request*/);

/** 위젯 초기화 델리게이트 */
DECLARE_DELEGATE_TwoParams(FOnConfigureWidgetForData, UUserWidget* /*Widget*/, const FUIExtensionRequest& /*Request*/);

/**
 * 동적 위젯 컨테이너 확장 포인트
 *
 * GameplayTag 기반 확장 시스템과 연동되어 동적으로 위젯을 추가/제거합니다.
 * UDynamicEntryBoxBase를 상속하여 여러 위젯을 관리할 수 있습니다.
 *
 * 사용 예시 (블루프린트):
 * 1. UMG에서 UIExtensionPointWidget을 배치합니다
 * 2. ExtensionPointTag를 설정합니다 (예: UI.ExtensionPoint.HUD.BottomLeft)
 * 3. DataClasses에 허용할 데이터 클래스를 추가합니다 (비워두면 모두 허용)
 */
UCLASS()
class COMMONUIEXTENSION_API UUIExtensionPointWidget : public UDynamicEntryBoxBase
{
	GENERATED_BODY()

public:
	UUIExtensionPointWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//-----------------------------------------------------------------------------
	// UWidget 오버라이드
	//-----------------------------------------------------------------------------

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual void SynchronizeProperties() override;
	virtual void BeginDestroy() override;

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	//-----------------------------------------------------------------------------
	// 확장 관리
	//-----------------------------------------------------------------------------

	/** 확장이 추가/제거되었을 때 호출됩니다 */
	void OnExtensionAction(EUIExtensionAction Action, const FUIExtensionRequest& Request);

	/** 확장 요청에 대한 위젯을 추가합니다 */
	void AddExtensionWidget(const FUIExtensionRequest& Request);

	/** 확장 요청에 대한 위젯을 제거합니다 */
	void RemoveExtensionWidget(const FUIExtensionRequest& Request);

	/** 요청에 대한 위젯 클래스를 결정합니다 */
	TSubclassOf<UUserWidget> GetWidgetClassForRequest(const FUIExtensionRequest& Request) const;

public:
	/** 위젯 클래스 결정 델리게이트 */
	FOnGetWidgetClassForData GetWidgetClassForData;

	/** 위젯 초기화 델리게이트 */
	FOnConfigureWidgetForData ConfigureWidgetForData;

protected:
	/** 확장 포인트 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Extension", meta = (Categories = "UI.ExtensionPoint"))
	FGameplayTag ExtensionPointTag;

	/** 매칭 규칙 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Extension")
	EUIExtensionPointMatch ExtensionPointTagMatch = EUIExtensionPointMatch::ExactMatch;

	/** 허용되는 데이터 클래스 목록 (비어있으면 모두 허용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Extension")
	TArray<TObjectPtr<UClass>> DataClasses;

	/** 확장 포인트 등록 핸들 */
	FUIExtensionPointHandle ExtensionPointHandle;

	/** 확장 ID → 생성된 위젯 매핑 */
	UPROPERTY(Transient)
	TMap<FUIExtensionHandle, TObjectPtr<UUserWidget>> ExtensionWidgets;
};
