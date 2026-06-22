// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"

#include "CommonUIExtensions.generated.h"

class ULocalPlayer;
class UCommonPrimaryGameLayout;
class UCommonActivatableWidget;
class UCommonUIManagerSubsystem;

/**
 * CommonUIExtension 헬퍼 함수 라이브러리
 *
 * 레이아웃 접근, 위젯 푸시 등 자주 사용하는 기능에 대한 정적 헬퍼 함수를 제공합니다.
 */
UCLASS()
class COMMONUIEXTENSION_API UCommonUIExtensions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	//-----------------------------------------------------------------------------
	// 서브시스템 접근
	//-----------------------------------------------------------------------------

	/** WorldContext에서 CommonUIManagerSubsystem을 가져옵니다 */
	UFUNCTION(BlueprintPure, Category = "Common UI|Extensions", meta = (WorldContext = "WorldContextObject"))
	static UCommonUIManagerSubsystem* GetUIManagerSubsystem(const UObject* WorldContextObject);

	//-----------------------------------------------------------------------------
	// 레이아웃 접근
	//-----------------------------------------------------------------------------

	/** 지정된 LocalPlayer의 PrimaryGameLayout을 반환합니다 */
	UFUNCTION(BlueprintPure, Category = "Common UI|Extensions", meta = (WorldContext = "WorldContextObject"))
	static UCommonPrimaryGameLayout* GetRootLayoutForPlayer(const UObject* WorldContextObject, const ULocalPlayer* LocalPlayer);

	/** 첫 번째 LocalPlayer의 PrimaryGameLayout을 반환합니다 */
	UFUNCTION(BlueprintPure, Category = "Common UI|Extensions", meta = (WorldContext = "WorldContextObject"))
	static UCommonPrimaryGameLayout* GetRootLayoutForPrimaryPlayer(const UObject* WorldContextObject);

	/** PlayerController로부터 해당 플레이어의 PrimaryGameLayout을 반환합니다 */
	UFUNCTION(BlueprintPure, Category = "Common UI|Extensions")
	static UCommonPrimaryGameLayout* GetRootLayoutForPlayerController(const APlayerController* PlayerController);

	//-----------------------------------------------------------------------------
	// 위젯 푸시 헬퍼
	//-----------------------------------------------------------------------------

	/**
	 * 첫 번째 플레이어의 레이어에 위젯을 푸시합니다
	 * @param WorldContextObject 월드 컨텍스트
	 * @param LayerTag 대상 레이어 태그
	 * @param WidgetClass 푸시할 위젯 클래스
	 * @return 생성된 위젯 인스턴스
	 */
	UFUNCTION(BlueprintCallable, Category = "Common UI|Extensions", meta = (WorldContext = "WorldContextObject", DeterminesOutputType = "WidgetClass"))
	static UCommonActivatableWidget* PushWidgetToLayerForPrimaryPlayer(const UObject* WorldContextObject, UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass);

	/**
	 * 지정된 플레이어의 레이어에 위젯을 푸시합니다
	 * @param LocalPlayer 대상 플레이어
	 * @param LayerTag 대상 레이어 태그
	 * @param WidgetClass 푸시할 위젯 클래스
	 * @return 생성된 위젯 인스턴스
	 */
	UFUNCTION(BlueprintCallable, Category = "Common UI|Extensions", meta = (DeterminesOutputType = "WidgetClass"))
	static UCommonActivatableWidget* PushWidgetToLayerForPlayer(const ULocalPlayer* LocalPlayer, UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass);
};
