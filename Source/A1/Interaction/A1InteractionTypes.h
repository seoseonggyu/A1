#pragma once

#include "GameplayTagContainer.h"
#include "UObject/ScriptInterface.h"
#include "A1InteractionTypes.generated.h"

class IA1Interactable;
class AActor;
class AController;

/**
 * FA1InteractionQuery
 *
 * 상호작용을 요청하는 주체(아바타/컨트롤러) 정보. Interactable이 옵션을 구성하거나
 * 상호작용 가능 여부를 판정할 때 참조한다.
 */
USTRUCT(BlueprintType)
struct FA1InteractionQuery
{
	GENERATED_BODY()

public:
	/** 상호작용을 요청한 아바타 액터(플레이어 캐릭터). */
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> RequestingAvatar;

	/** 상호작용을 요청한 컨트롤러. */
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AController> RequestingController;
};

/**
 * FA1InteractionOption
 *
 * Interactable이 제공하는 하나의 상호작용 정보. 표시(Title)와 발동 조건(근접 거리),
 * 도착 시 대상에게 보낼 GameplayEvent 태그, 외곽선 색 구분용 스텐실 값을 담는다.
 */
USTRUCT(BlueprintType)
struct FA1InteractionOption
{
	GENERATED_BODY()

public:
	/** 이 옵션을 제공한 Interactable. GatherInteractionOptions에서 채워도 되고, 호출측에서 채워도 된다. */
	UPROPERTY(BlueprintReadWrite)
	TScriptInterface<IA1Interactable> Interactable;

	/** 커서 툴팁/UI에 표시할 문구. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	/** 이 거리(cm) 이내로 접근하면 상호작용이 발동된다. (2D 평면 거리 기준) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InteractionRange = 150.f;

	/** 발동 시 대상 액터에게 보낼 GameplayEvent 태그. (BP/GA 확장용, 없으면 OnInteractAuth만 실행) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "GameplayEvent"))
	FGameplayTag InteractEventTag;

	/**
	 * 외곽선(CustomDepth) 스텐실 값. 대상 종류별 색 구분에 사용한다.
	 * 사용 중인 값: WorldInteractable 기본=1, Door=2, Pickup=3, Corpse=4, Character=5, ExtractionZone=6, LootContainer=7.
	 * 새 종류를 추가할 때는 겹치지 않는 값을 사용한다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HighlightStencil = 1;
};
