// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameStateComponent.h"
#include "Coro.h"
#include "ExperienceManagerComponent.generated.h"

class UExperienceDefinition;

DECLARE_LOG_CATEGORY_EXTERN(ExperienceManagerLog, Log, All);

/** Experience 로드 상태 */
UENUM(BlueprintType)
enum class EExperienceLoadState : uint8
{
	Unloaded,
	Loading,
	LoadingGameFeatures,
	Loaded,
	Deactivating
};

/** Experience 로드 완료 델리게이트 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnExperienceLoaded, const UExperienceDefinition*);

/**
 * Experience 로딩 및 동기화를 관리하는 GameState 컴포넌트
 *
 * GameState에 추가되어 Experience의 전체 생명주기를 관리합니다.
 * 서버에서 Experience를 설정하면 클라이언트에 자동으로 복제됩니다.
 *
 * 동작 방식:
 * - 서버: SetCurrentExperience() 호출 → Experience 로드 → GameFeature 로드
 * - 클라이언트: OnRep에서 ExperienceId 수신 → 동일하게 로드
 * - 양쪽: 로드 완료 시 OnExperienceLoaded 브로드캐스트
 */
UCLASS()
class COMMONGAME_API UExperienceManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	UExperienceManagerComponent(const FObjectInitializer& ObjectInitializer);

};