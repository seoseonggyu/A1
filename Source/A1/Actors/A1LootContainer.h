// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Pawn.h"
#include "Coro.h"
#include "Interaction/A1Interactable.h"
#include "A1LootContainer.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(A1LootContainerLog, Log, All);

class UStaticMeshComponent;
class UWidgetComponent;
class UInventoryComponent;
class UItemDefinition;

/**
 * FA1LootTableEntry
 *
 * AA1LootContainer 하나의 루트 테이블 항목. Weight가 클수록 더 자주 뽑힌다.
 */
USTRUCT(BlueprintType)
struct FA1LootTableEntry
{
	GENERATED_BODY()

public:
	/** 뽑힐 아이템 정의. */
	UPROPERTY(EditDefaultsOnly, Category = "Loot")
	TObjectPtr<const UItemDefinition> ItemDefinition;

	/** 뽑혔을 때 지급할 최소 수량. */
	UPROPERTY(EditDefaultsOnly, Category = "Loot", meta = (ClampMin = "1"))
	int32 MinCount = 1;

	/** 뽑혔을 때 지급할 최대 수량. */
	UPROPERTY(EditDefaultsOnly, Category = "Loot", meta = (ClampMin = "1"))
	int32 MaxCount = 1;

	/** 다른 항목 대비 상대적 가중치. 0이면 절대 뽑히지 않는다. */
	UPROPERTY(EditDefaultsOnly, Category = "Loot", meta = (ClampMin = "0.0"))
	float Weight = 1.f;
};

/**
 * AA1LootContainer
 *
 * 월드에 배치되는 파밍용 상자(APawn 기반). UInventoryComponent가 Pawn 전용이라 시체 루팅과
 * 동일한 UI/컴포넌트 파이프라인(UA1InventoryWidget, InventoryComponent 조건 없는 전체 복제)을
 * 그대로 재사용하기 위해 컨트롤되지 않는 Pawn으로 만든다.
 *
 * BeginPlay(서버)에서 LootTable을 가중치 랜덤으로 MinRolls~MaxRolls회 뽑아 InventoryComponent에
 * 채운다. 상호작용은 UA1Ability_Interact_LootContainer(GameplayEvent.Interact.LootContainer)가
 * 처리하며, 시체 루팅과 마찬가지로 UI를 열어 드래그로 하나씩 가져가는 방식이다(즉시 전량 지급 아님).
 */
UCLASS()
class A1_API AA1LootContainer : public APawn, public IA1Interactable
{
	GENERATED_BODY()

public:
	AA1LootContainer(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

	//-----------------------------------------------------------------------------
	// IA1Interactable
	//-----------------------------------------------------------------------------

	virtual void GetHighlightComponents(TArray<UPrimitiveComponent*>& OutComponents) const override;
	virtual void GatherInteractionOptions(const FA1InteractionQuery& Query, TArray<FA1InteractionOption>& OutOptions) const override;
	virtual bool CanInteract(const FA1InteractionQuery& Query) const override;
	virtual void SetInteractionPromptVisible(bool bVisible) override;

protected:
	virtual void BeginPlay() override;

private:
	/** 서버 전용. LootTable에서 가중치 랜덤으로 뽑아 InventoryComponent를 채운다. */
	TCoroTask<void> GenerateRandomLootAuthCoroutine();

protected:
	/** 상호작용 대상 메시. 호버 하이라이트/커서 트레이스 대상이자 루트 컴포넌트. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "A1|LootContainer")
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** 상호작용 프롬프트("루팅" 등) 위젯. AA1WorldInteractable과 동일한 용도. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "A1|LootContainer")
	TObjectPtr<UWidgetComponent> PromptWidgetComponent;

	/** 상자 내용물을 담는 인벤토리. 플레이어/시체와 동일하게 조건 없이 전체 복제된다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "A1|LootContainer")
	TObjectPtr<UInventoryComponent> InventoryComponent;

	/** 커서 툴팁/UI에 표시할 문구. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "A1|LootContainer")
	FText InteractionTitle;

	/** 이 거리(cm) 이내로 접근하면 상호작용이 발동된다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "A1|LootContainer")
	float InteractionRange = 150.f;

	/** 이 상자가 뽑을 수 있는 아이템 후보 목록. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|LootContainer|Loot")
	TArray<FA1LootTableEntry> LootTable;

	/** 스폰 시 뽑을 최소 횟수. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|LootContainer|Loot", meta = (ClampMin = "0"))
	int32 MinRolls = 1;

	/** 스폰 시 뽑을 최대 횟수. */
	UPROPERTY(EditDefaultsOnly, Category = "A1|LootContainer|Loot", meta = (ClampMin = "0"))
	int32 MaxRolls = 3;
};
