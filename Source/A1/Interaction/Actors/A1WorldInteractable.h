#pragma once

#include "GameFramework/Actor.h"
#include "Interaction/A1Interactable.h"
#include "A1WorldInteractable.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1WorldInteractableLog, Log, All);

class UStaticMeshComponent;

/**
 * AA1WorldInteractable
 *
 * IA1Interactable을 구현하는 월드 상호작용 액터의 공통 기반.
 * 스태틱 메시 하나를 가지며, EditDefaultsOnly 프로퍼티로 표시/발동 정보를 구성한다.
 *
 * - 호버 하이라이트 대상: Mesh (로컬 CustomDepth)
 * - 상호작용 결과 처리: OnInteractAuth (서버 전용). 하위 클래스가 오버라이드하거나
 *   BP에서 K2_OnInteractAuth로 처리한다.
 * - bConsumeOnUse=true면 1회 사용 후 bIsUsed(복제)가 true가 되어 더 이상 상호작용 불가.
 *
 * 커서 트레이스에 걸리도록 Mesh의 콜리전 프로파일 기본값을 "A1Interactable"로 둔다.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class A1_API AA1WorldInteractable : public AActor, public IA1Interactable
{
	GENERATED_BODY()

public:
	AA1WorldInteractable(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//-----------------------------------------------------------------------------
	// IA1Interactable
	//-----------------------------------------------------------------------------

	virtual void GetHighlightComponents(TArray<UPrimitiveComponent*>& OutComponents) const override;
	virtual void GatherInteractionOptions(const FA1InteractionQuery& Query, TArray<FA1InteractionOption>& OutOptions) const override;
	virtual bool CanInteract(const FA1InteractionQuery& Query) const override;
	virtual void OnInteractAuth(AActor* Interactor) override;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** BP에서 상호작용 결과 연출/처리를 확장하는 훅. 서버에서 호출된다. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnInteractAuth")
	void K2_OnInteractAuth(AActor* Interactor);

protected:
	/** 상호작용 대상 메시. 호버 하이라이트/커서 트레이스 대상. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "A1|Interactable")
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** 커서 툴팁/UI에 표시할 문구. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "A1|Interactable")
	FText InteractionTitle;

	/** 이 거리(cm) 이내로 접근하면 발동된다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "A1|Interactable")
	float InteractionRange = 150.f;

	/** 발동 시 대상에게 보낼 GameplayEvent 태그. (선택) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "A1|Interactable", meta = (Categories = "GameplayEvent"))
	FGameplayTag InteractEventTag;

	/** 외곽선 스텐실 값(종류별 색 구분). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "A1|Interactable")
	int32 HighlightStencil = 1;

	/** true면 1회 사용 후 더 이상 상호작용할 수 없다(bIsUsed로 표시). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "A1|Interactable")
	bool bConsumeOnUse = false;

	/** 사용 완료 여부(서버 권위, 복제). */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_bIsUsed, Category = "A1|Interactable")
	bool bIsUsed = false;

	UFUNCTION()
	virtual void OnRep_bIsUsed();
};
