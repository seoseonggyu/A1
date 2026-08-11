// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ViewModel/CommonViewModelBase.h"
#include "InteractionPromptViewModel.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(InteractionPromptViewModelLog, Log, All);

/**
 * UInteractionPromptViewModel
 *
 * 근접 상호작용 대상이 있을 때 화면에 띄우는 프롬프트("줍기" 등) 표시용 ViewModel.
 *
 * - 스캔 어빌리티(UA1Ability_Interact_Scan)가 대상 갱신 시 ShowPrompt/HidePrompt로 값을 채운다.
 * - HUD 위젯(BP)이 bIsVisible / PromptText에 MVVM 바인딩해 표시/숨김과 문구를 갱신한다.
 * - 순수 로컬 연출 데이터라 복제하지 않는다. (소유 클라의 PrimaryGameLayout에만 존재)
 */
UCLASS(BlueprintType)
class A1_API UInteractionPromptViewModel : public UCommonViewModelBase
{
	GENERATED_BODY()

public:
	/** 프롬프트를 표시하고 문구를 대상의 상호작용 문구(Title)로 갱신한다. */
	void ShowPrompt(const FText& InPromptText);

	/** 프롬프트를 숨긴다. */
	void HidePrompt();

	/** 프롬프트 표시 여부. */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Interaction ViewModel")
	bool bIsVisible = false;

	/** 표시할 상호작용 문구(예: "줍기"). */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Interaction ViewModel")
	FText PromptText;

public:
	/** ViewModel 이름 (MVVM 바인딩 식별자) */
	static const FName ViewModelName;
};
