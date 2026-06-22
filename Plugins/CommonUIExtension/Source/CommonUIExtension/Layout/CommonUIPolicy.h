// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UObject/Object.h"
#include "CommonUIPolicy.generated.h"

class UCommonLocalPlayer;
class UCommonPrimaryGameLayout;

DECLARE_LOG_CATEGORY_EXTERN(CommonUIPolicyLog, Log, All);

/**
 * 플레이어별 UI 레이아웃 정책
 *
 * 각 LocalPlayer에 대한 PrimaryGameLayout 생성 및 관리 정책을 정의합니다.
 * 게임 인스턴스에서 이 클래스를 서브클래싱하여 커스텀 레이아웃 생성 로직을 구현할 수 있습니다.
 *
 * 동작 방식:
 * - NotifyPlayerAdded 호출 시 OnPlayerControllerSet 델리게이트에 바인딩
 * - PlayerController가 설정되면 레이아웃 생성
 * - PlayerController가 없어도 델리게이트가 나중에 호출되어 레이아웃 생성
 */
UCLASS(Abstract, Blueprintable)
class COMMONUIEXTENSION_API UCommonUIPolicy : public UObject
{
	GENERATED_BODY()

public:
	UCommonUIPolicy(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//-----------------------------------------------------------------------------
	// 레이아웃 생성/관리
	//-----------------------------------------------------------------------------

	/**
	 * 지정된 LocalPlayer에 대한 PrimaryGameLayout을 가져옵니다
	 * 아직 생성되지 않았으면 nullptr을 반환합니다
	 */
	UCommonPrimaryGameLayout* GetRootLayout(const UCommonLocalPlayer* LocalPlayer) const;

	/**
	 * LocalPlayer가 추가되었을 때 호출됩니다
	 * OnPlayerControllerSet 델리게이트에 바인딩하여 PlayerController 설정 시 레이아웃을 생성합니다
	 */
	virtual void NotifyPlayerAdded(UCommonLocalPlayer* LocalPlayer);

	/**
	 * LocalPlayer가 제거되기 전에 호출됩니다
	 * 레이아웃을 뷰포트에서 제거하고 정리합니다
	 */
	virtual void NotifyPlayerRemoved(UCommonLocalPlayer* LocalPlayer);

protected:
	/** 레이아웃 위젯을 생성합니다 */
	void CreateLayoutWidget(UCommonLocalPlayer* LocalPlayer);

	/** 레이아웃을 뷰포트에 추가합니다 */
	void AddLayoutToViewport(UCommonLocalPlayer* LocalPlayer, UCommonPrimaryGameLayout* Layout) const;

	/** 레이아웃을 뷰포트에서 제거합니다 */
	void RemoveLayoutFromViewport(const UCommonLocalPlayer* LocalPlayer, UCommonPrimaryGameLayout* Layout) const;

protected:
	/** 사용할 PrimaryGameLayout 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Common UI|Policy")
	TSubclassOf<UCommonPrimaryGameLayout> LayoutWidgetClass;

	/** 플레이어별 생성된 레이아웃 캐시 */
	UPROPERTY(Transient)
	TMap<TObjectPtr<const UCommonLocalPlayer>, TObjectPtr<UCommonPrimaryGameLayout>> PlayerLayoutMap;
};