#pragma once

#include "Interaction/Actors/A1WorldInteractable.h"
#include "A1Interactable_Pickup.generated.h"

class UItemDefinition;
class UStaticMesh;
class USkeletalMesh;
class USkeletalMeshComponent;
class USphereComponent;

DECLARE_LOG_CATEGORY_EXTERN(A1InteractablePickupLog, Log, All);

/**
 * AA1Interactable_Pickup
 *
 * 바닥에 떨어진 아이템. 근접 상호작용 시 서버에서 Interactor의 InventoryComponent에
 * ItemDefinition을 추가한 뒤 액터를 제거한다.
 *
 * 드롭으로 스폰될 때는 원본 아이템(장비 정의)에서 찾은 메시를 실어받아 복제한다.
 * StaticMesh를 우선 사용하고(기본 Mesh 컴포넌트), 장비 액터가 SkeletalMeshComponent만
 * 갖는 경우엔 DisplaySkeletalMeshComponent로 대신 표시한다.
 *
 * InteractionCollision(SphereComponent)이 표시 메시 종류와 무관하게 항상 콜리전을 제공한다.
 * (StaticMesh가 비어 있으면 Mesh 자체엔 콜리전 형상이 없어, 이게 없으면 스캔/커서 트레이스에 전혀 안 잡힌다)
 */
UCLASS()
class A1_API AA1Interactable_Pickup : public AA1WorldInteractable
{
	GENERATED_BODY()

public:
	AA1Interactable_Pickup(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void OnInteractAuth(AActor* Interactor) override;

	/** 스폰 직후 이 픽업이 지급할 아이템 정보를 실어줄 때 사용한다. 서버 전용 */
	void InitializePickupDataAuth(const UItemDefinition* InItemDefinition, int32 InItemCount);

	/**
	 * 드롭 시 표시할 메시를 설정한다. 한쪽만 채워도 된다(StaticMesh 우선 표시).
	 * InDisplayScale은 원본 장비의 AttachTransform 스케일로, 손에 들었을 때와 동일한 크기로 보이게 한다.
	 * 복제되어 각 머신의 컴포넌트에도 동일하게 반영된다. 서버 전용.
	 */
	void SetDisplayMeshAuth(UStaticMesh* InStaticMesh, USkeletalMesh* InSkeletalMesh, const FVector& InDisplayScale);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 하이라이트(외곽선) 대상: SkeletalMesh를 쓰는 중이면 DisplaySkeletalMeshComponent를, 아니면 기본(Mesh)을 반환한다. */
	virtual void GetHighlightComponents(TArray<UPrimitiveComponent*>& OutComponents) const override;

	/** DisplayStaticMesh 복제 시 Mesh 컴포넌트에 반영한다. */
	UFUNCTION()
	void OnRep_DisplayStaticMesh();

	/** DisplaySkeletalMesh 복제 시 DisplaySkeletalMeshComponent에 반영한다. */
	UFUNCTION()
	void OnRep_DisplaySkeletalMesh();

	/** DisplayScale 복제 시 두 표시 컴포넌트 모두에 다시 반영한다(수신 순서와 무관하게 최종 일치시키기 위함). */
	UFUNCTION()
	void OnRep_DisplayScale();

protected:
	/** 이 픽업이 지급할 아이템 정의. 비어 있으면 인벤토리에 추가하지 않고 제거만 한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "A1|Interactable|Pickup")
	TObjectPtr<const UItemDefinition> ItemDefinition;

	/** 지급 수량. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "A1|Interactable|Pickup", meta = (ClampMin = "1"))
	int32 ItemCount = 1;

	/** 드롭 시 원본 아이템에서 찾은 표시용 StaticMesh(서버 권위, 복제). 비어 있으면 BP 기본 메시를 유지한다. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DisplayStaticMesh, Category = "A1|Interactable|Pickup")
	TObjectPtr<UStaticMesh> DisplayStaticMesh = nullptr;

	/** 드롭 시 원본 아이템에서 찾은 표시용 SkeletalMesh(서버 권위, 복제). StaticMesh가 없을 때만 사용한다. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DisplaySkeletalMesh, Category = "A1|Interactable|Pickup")
	TObjectPtr<USkeletalMesh> DisplaySkeletalMesh = nullptr;

	/**
	 * 표시 컴포넌트에 적용할 스케일(서버 권위, 복제). 원본 장비의 FEquipmentActorToSpawn::AttachTransform에서
	 * 가져온다. 기본값(1,1,1)은 BP에서 직접 배치한 픽업(EditAnywhere ItemDefinition 케이스)에 쓰인다.
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DisplayScale, Category = "A1|Interactable|Pickup")
	FVector DisplayScale = FVector::OneVector;

	/** DisplaySkeletalMesh를 보여줄 때 사용하는 보조 컴포넌트. 기본은 숨김. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "A1|Interactable|Pickup")
	TObjectPtr<USkeletalMeshComponent> DisplaySkeletalMeshComponent;

	/**
	 * 상호작용(스캔/커서) 감지 전용 콜리전. 표시 메시가 무엇이든(StaticMesh 없음/SkeletalMesh 콜리전 꺼짐)
	 * 항상 "A1Interactable" 프로파일로 존재해야 스캔의 OverlapMultiByChannel에 잡힌다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "A1|Interactable|Pickup")
	TObjectPtr<USphereComponent> InteractionCollision;
};
