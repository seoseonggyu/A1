// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Action/GameFeatureAction_WorldNetworkBase.h"
#include "Extension/ActorExtension.h"
#include "GameFeatureAction_AddActorExtension.generated.h"

struct FGameFeatureStateChangeContext;

/**
 * Actor에 ActorExtension을 추가하는 GameFeatureAction
 *
 * ActorExtensionWorldSubsystem에 Extension을 등록합니다.
 * bAddToSimulatedProxy로 SimulatedProxy 추가 여부를 제어합니다.
 *
 * 주의: ActorExtensionWorldSubsystem이 GFCM Handler를 Common* 클래스들에만 등록하므로,
 * TargetClass는 CommonCharacter, CommonPlayerController 등 Common 계열만 지원합니다.
 */
UCLASS(meta = (DisplayName = "Add Actor Extension"))
class COMMONGAME_API UGameFeatureAction_AddActorExtension : public UGameFeatureAction_WorldNetworkBase
{
	GENERATED_BODY()

public:
	//-----------------------------------------------------------------------------
	// UGameFeatureAction_WorldNetworkBase 오버라이드
	//-----------------------------------------------------------------------------

	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;

private:
	/** Subsystem에 Extension 등록 */
	void RegisterExtension(UWorld* World);

	/** Subsystem에서 Extension 해제 */
	void UnregisterExtension(UWorld* World);

public:
	/** 대상 Actor 클래스 */
	UPROPERTY(EditAnywhere, Category = "Extension", meta = (AllowAbstract = "false"))
	TSubclassOf<AActor> TargetClass;

	/** 해당 클래스에 추가할 Extension */
	UPROPERTY(EditAnywhere, Category = "Extension")
	FActorExtension Extension;

	/** LocallyControlled Pawn에 Extension을 추가할지 여부 */
	UPROPERTY(EditAnywhere, Category = "Network", meta = (EditCondition = "bClientAction"))
	uint8 bAddToLocallyControlled : 1 = false;

	/** SimulatedProxy에 Extension을 추가할지 여부 */
	UPROPERTY(EditAnywhere, Category = "Network", meta = (EditCondition = "bClientAction"))
	uint8 bAddToSimulatedProxy : 1 = false;
};
