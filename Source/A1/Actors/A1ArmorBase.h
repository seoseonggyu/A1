#pragma once

#include "Coro.h"

#include "A1ArmorBase.generated.h"

UCLASS(BlueprintType, Blueprintable)
class AA1ArmorBase : public AActor
{
	GENERATED_BODY()

public:
	AA1ArmorBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


public:
	void InitializeActor(TSoftObjectPtr<USkeletalMesh> InDefaultArmorMesh);

	USkeletalMeshComponent* GetMesh() const { return ArmorMeshComponent; }
	
private:
	TCoroTask<void> LoadArmorMeshCoroutine(TSoftObjectPtr<USkeletalMesh> ArmorMesh);



protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> ArmorMeshComponent;


};