#pragma once

#include "Interaction/Actors/A1WorldInteractable.h"
#include "A1Interactable_Pickup.generated.h"

/**
 * AA1Interactable_Pickup
 *
 * 바닥에 떨어진 아이템 예시. 근접 상호작용 시 서버에서 획득 처리 후 액터를 제거한다.
 * 실제 인벤토리 연동은 확장 지점(K2_OnInteractAuth 또는 아래 TODO)에서 처리한다.
 */
UCLASS()
class A1_API AA1Interactable_Pickup : public AA1WorldInteractable
{
	GENERATED_BODY()

public:
	AA1Interactable_Pickup(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void OnInteractAuth(AActor* Interactor) override;
};
