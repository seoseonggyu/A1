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
 * IA1Interactable을 구현해 다른 플레이어의 상호작용 대상이 될 수 있다.
 * CanInteract가 사망(Health<=0) 상태만 허용하므로, 시체 상태에서만 Interact가 가능하다.
 * 실제 결과 처리(장비/인벤토리 열람 등)는 GatherInteractionOptions가 제공하는
 * InteractEventTag(GameplayEvent.Interact.Player)로 UA1Ability_Interact_Player에 위임한다.
 */
UCLASS()
class A1_API AA1Character : public ACommonCharacter, public IA1Interactable
{
	GENERATED_BODY()

public:
	AA1Character(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	
	//-----------------------------------------------------------------------------
	// IA1Interactable
	//-----------------------------------------------------------------------------

	virtual void GetHighlightComponents(TArray<UPrimitiveComponent*>& OutComponents) const override;
	virtual void GatherInteractionOptions(const FA1InteractionQuery& Query, TArray<FA1InteractionOption>& OutOptions) const override;
	virtual bool CanInteract(const FA1InteractionQuery& Query) const override;
	
	void HandleDeathAuth();

};
