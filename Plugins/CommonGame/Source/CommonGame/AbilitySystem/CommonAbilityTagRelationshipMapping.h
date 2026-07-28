// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "CommonAbilityTagRelationshipMapping.generated.h"

/**
 * 어빌리티 태그 간의 관계(차단/취소/활성 조건)를 정의하는 항목.
 *
 * 하나의 AbilityTag를 기준으로, 해당 태그를 가진 어빌리티가
 * 다른 어빌리티를 어떻게 막고 취소하는지, 그리고 활성화에 필요한
 * 조건 태그를 암시적으로 어떻게 확장하는지를 서술한다.
 */
USTRUCT()
struct FCommonAbilityTagRelationship
{
	GENERATED_BODY()

	/** 이 관계가 기준으로 삼는 태그. 어빌리티는 이런 태그를 여러 개 가질 수 있다. */
	UPROPERTY(EditAnywhere, Category = Ability)
	FGameplayTag AbilityTag;

	/** 이 태그를 사용하는 어빌리티가 차단(Block)하는 다른 어빌리티 태그들 */
	UPROPERTY(EditAnywhere, Category = Ability)
	FGameplayTagContainer AbilityTagsToBlock;

	/** 이 태그를 사용하는 어빌리티가 취소(Cancel)하는 다른 어빌리티 태그들 */
	UPROPERTY(EditAnywhere, Category = Ability)
	FGameplayTagContainer AbilityTagsToCancel;

	/** 어빌리티가 이 태그를 가지면 활성화 필수(Required) 태그에 암시적으로 추가된다. */
	UPROPERTY(EditAnywhere, Category = Ability)
	FGameplayTagContainer ActivationRequiredTags;

	/** 어빌리티가 이 태그를 가지면 활성화 차단(Blocked) 태그에 암시적으로 추가된다. */
	UPROPERTY(EditAnywhere, Category = Ability)
	FGameplayTagContainer ActivationBlockedTags;
};


/**
 * 어빌리티 태그가 다른 어빌리티를 어떻게 차단/취소하는지 매핑하는 DataAsset.
 *
 * ASC가 어빌리티 활성화 시 이 매핑을 참조해 Block/Cancel 태그와
 * 추가 활성화 조건 태그를 계산하는 데 사용한다.
 */
UCLASS()
class COMMONGAME_API UCommonAbilityTagRelationshipMapping : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 주어진 어빌리티 태그 집합에 대해 차단/취소할 태그를 채워 반환한다. */
	void GetAbilityTagsToBlockAndCancel(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutTagsToBlock, FGameplayTagContainer* OutTagsToCancel) const;

	/** 주어진 어빌리티 태그 집합에 대해 추가되는 활성화 필수/차단 태그를 채워 반환한다. */
	void GetRequiredAndBlockedActivationTags(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutActivationRequired, FGameplayTagContainer* OutActivationBlocked) const;

	/** 지정한 ActionTag가 주어진 어빌리티 태그를 취소시키면 true를 반환한다. */
	bool IsAbilityCancelledByTag(const FGameplayTagContainer& AbilityTags, const FGameplayTag& ActionTag) const;

private:
	/** 서로 다른 게임플레이 태그 간의 차단/취소 관계 목록 */
	UPROPERTY(EditAnywhere, Category = Ability, meta = (TitleProperty = "AbilityTag"))
	TArray<FCommonAbilityTagRelationship> AbilityTagRelationships;
};
