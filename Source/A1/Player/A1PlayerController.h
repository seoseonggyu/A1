// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/CommonPlayerController.h"
#include "A1PlayerController.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1PlayerControllerLog, Log, All);


/**
 * PlayerController
 *
 * �÷��̾��� �� ������ �����ϰ�, ITeamInterface�� �����մϴ�.
 * �� ������ �������� �����Ǹ� Ŭ���̾�Ʈ�� �����˴ϴ�.
 */
UCLASS()
class A1_API AA1PlayerController : public ACommonPlayerController
{
	GENERATED_BODY()

public:
	AA1PlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//-----------------------------------------------------------------------------
	// AActor �������̵�
	//-----------------------------------------------------------------------------

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//-----------------------------------------------------------------------------
	// 상호작용 호버 (로컬 전용)
	//-----------------------------------------------------------------------------

	/** 현재 커서가 올라가 있는 Interactable 액터. 소유 클라에서만 유효하며 복제되지 않는다. */
	AActor* GetHoveredInteractable() const { return HoveredInteractable.Get(); }

protected:
	/** 매 프레임 입력 처리 시점. 소유 클라에서 커서 호버를 갱신한다. */
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;

private:
	/** 커서 아래 Interactable을 트레이스해 하이라이트를 갱신한다. 로컬 전용. */
	void UpdateInteractionHoverLocal();

	/** 대상 Interactable의 하이라이트(CustomDepth 외곽선)를 켜고 끈다. 로컬 전용. */
	void SetInteractableHighlightLocal(AActor* InteractableActor, bool bHighlight);

	/** 현재 호버 중인 Interactable. 비복제. */
	TWeakObjectPtr<AActor> HoveredInteractable;
	
};
