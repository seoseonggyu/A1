// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/CommonCharacter.h"
#include "Interaction/A1Interactable.h"
#include "A1Character.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1CharacterLog, Log, All);


/**
 *
 * �� �ý����� �����ϴ� ĳ���� Ŭ�����Դϴ�.
 * �� ������ ���� PlayerController���� �����ɴϴ�.
 *
 * IA1Interactable을 구현해 다른 플레이어의 커서 호버 대상이 될 수 있다 (하이라이트 전용,
 * 이번 단계에서는 실제 상호작용 처리(OnInteractAuth)는 연결하지 않는다).
 */
UCLASS()
class A1_API AA1Character : public ACommonCharacter, public IA1Interactable
{
	GENERATED_BODY()

public:
	AA1Character(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//-----------------------------------------------------------------------------
	// IA1Interactable (하이라이트 전용)
	//-----------------------------------------------------------------------------

	virtual void GetHighlightComponents(TArray<UPrimitiveComponent*>& OutComponents) const override;
	virtual void GatherInteractionOptions(const FA1InteractionQuery& Query, TArray<FA1InteractionOption>& OutOptions) const override;
	virtual bool CanInteract(const FA1InteractionQuery& Query) const override;

};
