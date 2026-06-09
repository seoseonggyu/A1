// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Action/GameFeatureAction_WorldNetworkBase.h"
#include "AbilitySystem/Ability/CommonAbilityTypes.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameFeatureAction_AddAbilities.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(GameFeatureAction_AddAbilitiesLog, Log, All);

class UCommonAbilitySystemComponent;
struct FComponentRequestHandle;

//-----------------------------------------------------------------------------
// FAbilityGrantedHandles
//-----------------------------------------------------------------------------

/**
 * Context별 부여된 Ability 핸들 관리
 */
struct FAbilityGrantedHandles
{
public:
	/** GFCM Extension 핸들 */
	TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle;

	/** Actor별 부여된 Ability 핸들 */
	TMap<TWeakObjectPtr<AActor>, TArray<FGameplayAbilitySpecHandle>> AbilitySpecHandles;
};

//-----------------------------------------------------------------------------
// UGameFeatureAction_AddAbilities
//-----------------------------------------------------------------------------

/**
 * Actor에 Ability를 추가하는 GameFeatureAction
 *
 * GFCM을 통해 대상 Actor 스폰 시 Ability를 부여합니다.
 * InputTag가 지정된 경우 DynamicSpecSourceTags에 추가되어
 * 입력 시스템과 연동됩니다.
 */
UCLASS(meta = (DisplayName = "Add Abilities"))
class COMMONGAME_API UGameFeatureAction_AddAbilities : public UGameFeatureAction_WorldNetworkBase
{
	GENERATED_BODY()

public:
	UGameFeatureAction_AddAbilities();

	//-----------------------------------------------------------------------------
	// UGameFeatureAction_WorldNetworkBase 오버라이드
	//-----------------------------------------------------------------------------

	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;

private:
	/** Actor에 Ability 부여 */
	void HandleActorExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext);
	
	/** Actor의 ASC 가져오기 */
	UCommonAbilitySystemComponent* GetAbilitySystemComponent(AActor* Actor) const;

public:
	/** 대상 Actor 클래스 (서버에서 필터링용) */
	UPROPERTY(EditAnywhere, Category = "Abilities", meta = (AssetBundles = "Server"))
	TSoftClassPtr<AActor> TargetClass;

	/** 부여할 Ability 목록 */
	UPROPERTY(EditAnywhere, Category = "Abilities", meta = (TitleProperty = "InputTag"))
	TArray<FCommonAbilityEntry> Abilities;

private:
	/** Context별 핸들 관리 */
	TMap<FGameFeatureStateChangeContext, FAbilityGrantedHandles> ContextHandles;
};
