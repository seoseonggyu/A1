// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"

#include "CommonUIManagerSubsystem.generated.h"

class UCommonUIPolicy;
class UCommonPrimaryGameLayout;
class UCommonLocalPlayer;

DECLARE_LOG_CATEGORY_EXTERN(CommonUIManagerSubsystemLog, Log, All);

/**
 * GameInstance 레벨 UI 관리 서브시스템
 *
 * UCommonUIPolicy를 통해 LocalPlayer별 PrimaryGameLayout을 관리합니다.
 * 플레이어 추가/제거 시 자동으로 레이아웃을 생성/정리합니다.
 * 데디케이티드 서버에서는 생성되지 않습니다.
 *
 * 주의: UCommonLocalPlayer를 사용해야 합니다.
 */
UCLASS(Config = Game)
class COMMONUIEXTENSION_API UCommonUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UCommonUIManagerSubsystem();

	//-----------------------------------------------------------------------------
	// USubsystem 오버라이드
	//-----------------------------------------------------------------------------

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	//-----------------------------------------------------------------------------
	// UI Policy 관리
	//-----------------------------------------------------------------------------

	/** 현재 UI Policy를 반환합니다 */
	UFUNCTION(BlueprintPure, Category = "Common UI|Manager")
	UCommonUIPolicy* GetCurrentUIPolicy() const { return CurrentPolicy; }

	/** UI Policy를 설정합니다 */
	UFUNCTION(BlueprintCallable, Category = "Common UI|Manager")
	void SetCurrentUIPolicy(TSubclassOf<UCommonUIPolicy> PolicyClass);

	//-----------------------------------------------------------------------------
	// 레이아웃 접근
	//-----------------------------------------------------------------------------

	/** 지정된 LocalPlayer의 PrimaryGameLayout을 반환합니다 */
	UFUNCTION(BlueprintPure, Category = "Common UI|Manager")
	UCommonPrimaryGameLayout* GetRootLayoutForPlayer(const ULocalPlayer* LocalPlayer) const;

	/** 첫 번째 LocalPlayer의 PrimaryGameLayout을 반환합니다 */
	UFUNCTION(BlueprintPure, Category = "Common UI|Manager")
	UCommonPrimaryGameLayout* GetRootLayoutForPrimaryPlayer() const;

protected:
	/** LocalPlayer가 추가되었을 때 호출됩니다 */
	void OnLocalPlayerAdded(ULocalPlayer* LocalPlayer) const;

	/** LocalPlayer가 제거되기 전에 호출됩니다 */
	void OnLocalPlayerRemoved(ULocalPlayer* LocalPlayer) const;

protected:
	/** 현재 활성화된 UI Policy */
	UPROPERTY(Transient)
	TObjectPtr<UCommonUIPolicy> CurrentPolicy;

	/**
	 * 기본 UI Policy 클래스
	 *
	 * DefaultGame.ini에서 설정:
	 * [/Script/CommonUIExtension.CommonUIManagerSubsystem]
	 * DefaultUIPolicyClass=/Game/UI/BP_UIPolicy.BP_UIPolicy_C
	 */
	UPROPERTY(Config)
	TSoftClassPtr<UCommonUIPolicy> DefaultUIPolicyClass;

	/** LocalPlayer 이벤트 델리게이트 핸들 */
	FDelegateHandle LocalPlayerAddedHandle;
	FDelegateHandle LocalPlayerRemovedHandle;
};