// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/CommonPlayerController.h"
#include "A1PlayerController.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1PlayerControllerLog, Log, All);

// TODO: 팀 변경
/** 팀 변경 시 호출되는 델리게이트 */
//DECLARE_MULTICAST_DELEGATE_TwoParams(FOnTeamChanged, ACSPlayerController* /*PlayerController*/, FGameplayTag /*NewTeamTag*/);

/**
 * PlayerController
 *
 * 플레이어의 팀 정보를 관리하고, ITeamInterface를 구현합니다.
 * 팀 정보는 서버에서 설정되며 클라이언트로 복제됩니다.
 */
UCLASS()
class A1_API AA1PlayerController : public ACommonPlayerController
{
	GENERATED_BODY()

public:
	AA1PlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//-----------------------------------------------------------------------------
	// AActor 오버라이드
	//-----------------------------------------------------------------------------

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//-----------------------------------------------------------------------------
	// ITeamInterface 구현
	//-----------------------------------------------------------------------------

	// TODO: 팀 관련
	// /virtual FGameplayTag GetTeamTag() const override { return TeamTag; }
	//virtual void SetTeamTagAuth(FGameplayTag NewTeamTag) override;
//
//protected:
//	/** 팀 태그 복제 시 호출 */
//	UFUNCTION()
//	void OnRep_TeamTag();
//
//public:
//	/** 팀 변경 델리게이트 */
//	FOnTeamChanged OnTeamChanged;
//
//private:
//	/** 현재 팀 태그 (복제됨) */
//	UPROPERTY(ReplicatedUsing = OnRep_TeamTag)
//	FGameplayTag TeamTag;
};
