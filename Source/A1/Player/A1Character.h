// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/CommonCharacter.h"
#include "A1Character.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1CharacterLog, Log, All);


/**
 *
 * 팀 시스템을 지원하는 캐릭터 클래스입니다.
 * 팀 정보는 소유 PlayerController에서 가져옵니다.
 */
UCLASS()
class A1_API AA1Character : public ACommonCharacter
{
	GENERATED_BODY()
	
public:
	AA1Character(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//-----------------------------------------------------------------------------
	// ITeamInterface 구현
	//-----------------------------------------------------------------------------

	// TODO: Team 관련
	/** 소유 PlayerController의 팀 태그를 반환합니다 */
	//virtual FGameplayTag GetTeamTag() const override; 

	/** Character에서는 팀 설정을 지원하지 않습니다 (PlayerController를 통해 설정) */
	//virtual void SetTeamTagAuth(FGameplayTag NewTeamTag) override;

	//-----------------------------------------------------------------------------
	// 죽음 처리
	//-----------------------------------------------------------------------------

	// TODO: 죽음 처리
	/** 체력이 0이 되었을 때 호출됩니다 (서버 전용) */
	//void HandleDeathAuth();
};
