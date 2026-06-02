// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "Extension/Condition/ExtensionCondition.h"
#include "Extension/Execute/ExtensionExecute.h"
#include "ActorExtension.generated.h"

/**
 * Actor 기능 확장을 위한 구조체
 *
 * TInstancedStruct를 사용하여 다형적인 Condition/Execute를 지원합니다.
 * 각 Actor마다 독립적인 복사본을 소유하므로 런타임 상태가 서로 간섭하지 않습니다.
 *
 * Conditions: 활성화 조건 목록 (AND 평가)
 * Executes: 활성화 시 실행할 작업 목록
 *
 * Component vs Extension:
 * - Component: Actor에 구조적으로 부착되며, 상태/데이터를 저장하고 Replication 가능
 * - Extension: Actor를 수정하지 않고 조건부로 행위만 주입
 *
 * Extension의 핵심 가치:
 * - 조건 충족 시에만 활성화 (네트워크 Ready, 특정 컴포넌트 존재 등)
 * - Actor 구조 변경 없이 GameFeature/Experience별로 다른 행위 구성
 * - Condition/Execute 조합을 데이터로 관리하여 코드 수정 없이 확장
 */
USTRUCT(BlueprintType)
struct COMMONGAME_API FActorExtension
{
	GENERATED_BODY()

public:
	/** 모든 조건이 충족되었는지 확인합니다 */
	bool CanActivate(AActor* Owner) const;

	/** 활성화 시 모든 Execute를 실행합니다 */
	void OnActivate(AActor* Owner);

	/** 비활성화 시 모든 Execute를 실행합니다 */
	void OnDeactivate(AActor* Owner);

	/** 활성화 상태를 반환합니다 */
	bool IsActivated() const { return bActivated; }

public:
	/** 활성화 조건 목록 (모두 AND로 평가) */
	UPROPERTY(EditAnywhere, Category = "Extension")
	TArray<TInstancedStruct<FExtensionCondition>> Conditions;

	/** 조건 충족 시 실행할 작업 목록 */
	UPROPERTY(EditAnywhere, Category = "Extension")
	TArray<TInstancedStruct<FExtensionExecute>> Executes;

private:
	/** 활성화 상태 (런타임 전용, 각 Actor마다 독립적) */
	bool bActivated = false;
};
