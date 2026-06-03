// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Extension/Condition/ExtensionCondition.h"
#include "ExtensionCondition_NetworkReady.generated.h"

class UAttributeSet;

/**
 * Controller와 PlayerState가 준비되었는지 확인하는 Condition
 *
 * 네트워크 환경에서 Pawn이 완전히 초기화되었는지 확인합니다.
 * Pawn이 아닌 Actor에 적용하면 항상 false를 반환합니다.
 */
USTRUCT(BlueprintType, meta = (DisplayName = "Network Ready"))
struct COMMONGAME_API FExtensionCondition_NetworkReady : public FExtensionCondition
{
	GENERATED_BODY()

public:
	virtual bool IsSatisfied(AActor* Owner) const override;

public:
	/** true면 PlayerController만 유효, false면 모든 Controller 허용 */
	UPROPERTY(EditAnywhere, Category = "Condition")
	uint8 bRequirePlayerController : 1 = true;

	/** PlayerState와 PlayerController가 연결되어 있는지 확인합니다 */
	UPROPERTY(EditAnywhere, Category = "Condition")
	uint8 bRequirePlayerStateLinked : 1 = true;

	/** PlayerState의 AbilitySystemComponent가 복제되었는지 확인합니다 */
	UPROPERTY(EditAnywhere, Category = "Condition")
	uint8 bRequireAbilitySystemReady : 1 = true;

	/** 복제되어야 하는 AttributeSet 클래스 목록 (비어있으면 체크 안 함) */
	UPROPERTY(EditAnywhere, Category = "Condition", meta = (EditCondition = "bRequireAbilitySystemReady"))
	TArray<TSubclassOf<UAttributeSet>> RequiredAttributeSets;
};
