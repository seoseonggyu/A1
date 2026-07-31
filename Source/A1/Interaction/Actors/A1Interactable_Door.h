#pragma once

#include "Interaction/Actors/A1WorldInteractable.h"
#include "A1Interactable_Door.generated.h"

/**
 * AA1Interactable_Door
 *
 * 상호작용으로 여닫는 문 예시. 서버 권위로 bIsOpen(복제)을 토글하고,
 * OnRep(및 서버)에서 Mesh를 회전시켜 열림/닫힘을 연출한다.
 * 반복 사용 가능하므로 bConsumeOnUse는 사용하지 않는다.
 */
UCLASS()
class A1_API AA1Interactable_Door : public AA1WorldInteractable
{
	GENERATED_BODY()

public:
	AA1Interactable_Door(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void OnInteractAuth(AActor* Interactor) override;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 열림 상태에 맞춰 Mesh의 Yaw를 적용한다. (서버·클라 공통 연출) */
	void ApplyDoorVisual();

	UFUNCTION()
	void OnRep_bIsOpen();

protected:
	/** 문이 열려 있는지 여부(서버 권위, 복제). */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_bIsOpen, Category = "A1|Door")
	bool bIsOpen = false;

	/** 열렸을 때 Mesh에 적용할 상대 Yaw(도). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "A1|Door")
	float OpenYaw = 90.f;

	/** 닫혔을 때의 기준 상대 Yaw(도). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "A1|Door")
	float ClosedYaw = 0.f;
};
