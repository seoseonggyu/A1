#pragma once

#include "A1Define.h"
#include "Components/PawnComponent.h"
#include "A1CosmeticManagerComponent.generated.h"

class AA1ArmorBase;

UCLASS(BlueprintType, Blueprintable)
class UA1CosmeticManagerComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UA1CosmeticManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Pawn에서 A1CosmeticManagerComponent를 찾아 반환합니다 */
	UFUNCTION(BlueprintCallable, Category = "Cosmetic")
	static UA1CosmeticManagerComponent* FindCosmeticManagerComponent(const APawn* Pawn);

	const TArray<TObjectPtr<UChildActorComponent>>& GetCosmeticSlots() const { return CosmeticSlots; }
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void InitializeManager();

	UChildActorComponent* SpawnCosmeticSlotActor(TSoftObjectPtr<USkeletalMesh> InDefaultMesh);

protected:
	UPROPERTY(EditDefaultsOnly, meta = (ArraySizeEnum = "EArmorType"))
	TSoftObjectPtr<USkeletalMesh> InitialCosmetics[(int32)EArmorType::Count];
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AA1ArmorBase> CosmeticSlotClass;

private:
	UPROPERTY()
	TArray<TObjectPtr<UChildActorComponent>> CosmeticSlots;

	bool bInitialized = false;
};