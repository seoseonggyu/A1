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

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void InitializeManager();

	UChildActorComponent* SpawnCosmeticSlotActor(TSoftObjectPtr<USkeletalMesh> InDefaultMesh);

protected:
	UPROPERTY(EditDefaultsOnly, meta = (ArraySizeEnum = "EArmorType"))
	TSoftObjectPtr<USkeletalMesh> InitialCosmetics[(int32)EArmorType::Count];

	/*UPROPERTY(EditDefaultsOnly) // TODO: 스킨 관련
	ECharacterSkinType CharacterSkinType = ECharacterSkinType::Asian;*/

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AA1ArmorBase> CosmeticSlotClass;

private:
	UPROPERTY()
	TArray<TObjectPtr<UChildActorComponent>> CosmeticSlots;

	bool bInitialized = false;
};