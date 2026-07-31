#pragma once

#include "Interaction/Actors/A1WorldInteractable.h"
#include "A1Interactable_Corpse.generated.h"

/**
 * AA1Interactable_Corpse
 *
 * 시체(루팅) 예시. 근접 상호작용 시 서버에서 루팅 처리 훅(K2_OnInteractAuth)을 호출한다.
 * 반복 루팅(창 열기)을 허용하기 위해 기본은 비소모형이다. (한 번만 열리게 하려면 bConsumeOnUse=true)
 */
UCLASS()
class A1_API AA1Interactable_Corpse : public AA1WorldInteractable
{
	GENERATED_BODY()

public:
	AA1Interactable_Corpse(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
