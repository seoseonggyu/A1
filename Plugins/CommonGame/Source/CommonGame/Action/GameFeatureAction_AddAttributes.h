// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Action/GameFeatureAction_WorldNetworkBase.h"
#include "GameFeatureAction_AddAttributes.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(GameFeatureAction_AddAttributesLog, Log, All);

class UAttributeSet;
class UCommonAbilitySystemComponent;
struct FComponentRequestHandle;

//-----------------------------------------------------------------------------
// FAttributeSetGrantedHandles
//-----------------------------------------------------------------------------

/**
 * Context별 부여된 AttributeSet 핸들 관리
 */
struct FAttributeSetGrantedHandles
{
public:
	/** GFCM Extension 핸들 */
	TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle;

	/** Actor별 부여된 AttributeSet */
	TMap<TWeakObjectPtr<AActor>, TArray<TWeakObjectPtr<UAttributeSet>>> AttributeSets;
};

//-----------------------------------------------------------------------------
// UGameFeatureAction_AddAttributes
//-----------------------------------------------------------------------------

/**
 * Actor에 AttributeSet을 추가하는 GameFeatureAction
 *
 * GFCM을 통해 대상 Actor 스폰 시 지정된 AttributeSet을 ASC에 추가합니다.
 * GameFeature 비활성화 시 추가된 AttributeSet을 제거합니다.
 */
UCLASS(meta = (DisplayName = "Add Attributes"))
class COMMONGAME_API UGameFeatureAction_AddAttributes : public UGameFeatureAction_WorldNetworkBase
{
	GENERATED_BODY()

public:
	UGameFeatureAction_AddAttributes();

	//-----------------------------------------------------------------------------
	// UGameFeatureAction_WorldNetworkBase 오버라이드
	//-----------------------------------------------------------------------------

	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;

private:
	/** Actor에 AttributeSet 부여 */
	void HandleActorExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext);

	/** Actor의 ASC 가져오기 */
	UCommonAbilitySystemComponent* GetAbilitySystemComponent(const AActor* Actor) const;

public:
	/** 대상 Actor 클래스 */
	UPROPERTY(EditAnywhere, Category = "Attributes", meta = (AssetBundles = "Server"))
	TSoftClassPtr<AActor> TargetClass;

	/** 추가할 AttributeSet 클래스 목록 */
	UPROPERTY(EditAnywhere, Category = "Attributes")
	TArray<TSubclassOf<UAttributeSet>> AttributeSetClasses;

private:
	/** Context별 핸들 관리 */
	TMap<FGameFeatureStateChangeContext, FAttributeSetGrantedHandles> ContextHandles;
};
