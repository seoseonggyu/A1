#pragma once

#include "UObject/Interface.h"
#include "A1InteractionTypes.h"
#include "A1Interactable.generated.h"

class UPrimitiveComponent;

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UA1Interactable : public UInterface
{
	GENERATED_BODY()
};

/**
 * IA1Interactable
 *
 * 마우스 커서 호버 하이라이트와 근접 상호작용의 대상이 되는 액터가 구현하는 인터페이스.
 * (몬스터, 캐릭터, 시체, 아이템, 문 등)
 *
 * - GetHighlightComponents : 외곽선 표시 대상. 로컬(소유 클라)에서만 사용된다.
 * - GatherInteractionOptions : 표시/발동 정보를 채운다. 최소 1개면 상호작용 가능.
 * - CanInteract : 상호작용 가능 여부. 로컬 사전 판정 + 서버 재검증에 모두 쓰인다.
 * - OnInteractAuth : 실제 결과 처리. 반드시 서버(Authority)에서만 호출된다.
 */
class A1_API IA1Interactable
{
	GENERATED_BODY()

public:
	/** 외곽선(CustomDepth)을 적용할 컴포넌트들을 수집한다. 로컬 하이라이트 전용. */
	virtual void GetHighlightComponents(TArray<UPrimitiveComponent*>& OutComponents) const {}

	/** 표시/발동 정보를 채운다. 하나 이상 채우면 상호작용 대상으로 취급된다. */
	virtual void GatherInteractionOptions(const FA1InteractionQuery& Query, TArray<FA1InteractionOption>& OutOptions) const {}

	/** 현재 상호작용이 가능한지 여부. (예: 이미 사용된 문/아이템은 false) */
	virtual bool CanInteract(const FA1InteractionQuery& Query) const { return true; }

	/**
	 * 실제 상호작용 결과를 처리한다. 문 열기, 아이템 획득, 데미지 등.
	 * 반드시 서버(Authority)에서만 호출된다. (C++ 인터페이스라 UFUNCTION 미부여, 호출측이 권한을 보장)
	 */
	virtual void OnInteractAuth(AActor* Interactor) {}
};
