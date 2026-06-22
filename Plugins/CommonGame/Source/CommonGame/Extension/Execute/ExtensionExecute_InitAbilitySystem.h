// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Extension/Execute/ExtensionExecute.h"
#include "ExtensionExecute_InitAbilitySystem.generated.h"

/**
 * AbilitySystem 초기화를 담당하는 Execute
 *
 * PlayerState의 ASC에 InitAbilityActorInfo를 호출하고,
 * PlayerController의 OnPostProcessInput에 ASC의 ProcessAbilityInput을 바인딩합니다.
 * Pawn이 아닌 Actor에 적용하면 동작하지 않습니다.
 */
USTRUCT(BlueprintType, meta = (DisplayName = "Init Ability System"))
struct COMMONGAME_API FExtensionExecute_InitAbilitySystem : public FExtensionExecute
{
	GENERATED_BODY()

public:
	virtual void OnActivate(AActor* Owner) const override;
	virtual void OnDeactivate(AActor* Owner) const override;

private:
	/** ProcessAbilityInput 델리게이트 핸들 (런타임 상태) */
	mutable FDelegateHandle ProcessInputDelegateHandle;
};
