// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CommonAbilityTypes.generated.h"

class UCommonGameplayAbility;
class UTexture2D;

//-----------------------------------------------------------------------------
// FCommonAbilityEntry
//-----------------------------------------------------------------------------

/**
 * 부여할 Ability 정보
 *
 * Ability 클래스와 선택적 입력 태그를 포함합니다.
 * InputTag가 지정되면 DynamicSpecSourceTags에 추가되어 입력 시스템과 연동됩니다.
 */
USTRUCT(BlueprintType)
struct COMMONGAME_API FCommonAbilityEntry
{
	GENERATED_BODY()

public:
	/** Ability 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UCommonGameplayAbility> Ability;

	/** 입력 태그 (DynamicSpecSourceTags에 추가됨) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (Categories = "Input"))
	FGameplayTag InputTag;

	/** 이 Ability를 UI에 표시할 때 쓸 아이콘 (예: 스킬 쿨타임 UI). UItemDefinition::Icon과 동일하게 장비 Definition에서 데이터로 지정 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UTexture2D> Icon = nullptr;
};

//-----------------------------------------------------------------------------
// ECommonAbilityActivationPolicy
//-----------------------------------------------------------------------------

/**
 * Ability 활성화 정책
 *
 * 입력에 따른 Ability 활성화 시점을 정의합니다.
 */
UENUM(BlueprintType)
enum class ECommonAbilityActivationPolicy : uint8
{
	Manual,
	
	/** 입력 시작 시 활성화 (누르는 순간) */
	OnInputTriggered,

	/** 입력 유지 중 활성화 (누르고 있는 동안 계속) */
	WhileInputActive,

	/** 스폰 시 자동 활성화 (패시브) */
	OnSpawn
};

//-----------------------------------------------------------------------------
// ECommonAbilityActivationGroup
//-----------------------------------------------------------------------------

/**
 * Ability 활성화 그룹
 *
 * Exclusive 그룹은 동시에 하나만 활성화됩니다.
 * - Blocking 활성화 중 → 모든 Exclusive 진입 차단
 * - Replaceable 활성화 중 → 새 Exclusive 진입 시 기존 취소
 * - Independent는 다른 그룹에 영향 없음
 */
UENUM(BlueprintType)
enum class ECommonAbilityActivationGroup : uint8
{
	/** 항상 활성화 가능, 다른 그룹에 영향 없음 */
	Independent,

	/** 새 Exclusive 진입 시 취소됨 */
	Exclusive_Replaceable,

	/** 활성화 중 모든 Exclusive 진입 차단 */
	Exclusive_Blocking,

	/** 카운트용 (사용 금지) */
	MAX UMETA(Hidden)
};