// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Extension/Execute/ExtensionExecute.h"
#include "Input/InputTypes.h"
#include "ExtensionExecute_BindInput_TopDown.generated.h"

struct FInputActionValue;
class UInputMappingContext;

/**
 * TopDown 스타일 입력 바인딩을 담당하는 Execute
 *
 * MappingContext를 등록하고, Native/Ability Input Action을 바인딩합니다.
 * Pawn이 아닌 Actor에 적용하면 동작하지 않습니다.
 */
USTRUCT(BlueprintType, meta = (DisplayName = "Bind Input (TopDown)"))
struct COMMONGAME_API FExtensionExecute_BindInput_TopDown : public FExtensionExecute
{
	GENERATED_BODY()

public:
	virtual void OnActivate(AActor* Owner) const override;
	virtual void OnDeactivate(AActor* Owner) const override;

private:
	/** 이동 입력 처리 */
	void Input_Move(const FInputActionValue& InputActionValue) const;

	/** 시점 입력 처리 */
	void Input_Look(const FInputActionValue& InputActionValue) const;

	/** Ability 입력 Press 처리 - ASC로 라우팅 */
	void Input_AbilityPressed(FGameplayTag InputTag) const;

	/** Ability 입력 Release 처리 - ASC로 라우팅 */
	void Input_AbilityReleased(FGameplayTag InputTag) const;

public:
	//-----------------------------------------------------------------------------
	// Mapping Context 설정
	//-----------------------------------------------------------------------------

	/** Input Mapping Context */
	UPROPERTY(EditAnywhere, Category = "Mapping Context", meta = (AssetBundles = "Client"))
	TSoftObjectPtr<UInputMappingContext> MappingContext;

	/** Mapping Context 우선순위 */
	UPROPERTY(EditAnywhere, Category = "Mapping Context")
	int32 Priority = 0;

	//-----------------------------------------------------------------------------
	// Input Actions 설정
	//-----------------------------------------------------------------------------

	/** Native Input Actions (이동 등) */
	UPROPERTY(EditAnywhere, Category = "Input Actions", meta = (TitleProperty = "{InputTag}"))
	TArray<FInputActionAndTag> NativeInputActions;

	/** Ability Input Actions (스킬 발동 등) */
	UPROPERTY(EditAnywhere, Category = "Input Actions", meta = (TitleProperty = "{InputTag}"))
	TArray<FInputActionAndTag> AbilityInputActions;

private:
	/** 활성화된 Pawn (런타임 상태) */
	mutable TWeakObjectPtr<APawn> WeakPawn;

	/** 바인딩 핸들 (런타임 상태) */
	mutable TArray<uint32> BindingHandles;
};
