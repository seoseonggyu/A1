// #pragma once
//
// #include "A1Define.h"
// #include "Abilities/GameplayAbilityTypes.h"
// #include "A1AnimNotify_SendWeaponEvent.generated.h"
//
// class AA1EquipmentBase;
//
// UCLASS(meta=(DisplayName="Send Weapon Event"))
// class UA1AnimNotify_SendWeaponEvent : public UAnimNotify
// {
// 	GENERATED_BODY()
// 	
// public:
// 	UA1AnimNotify_SendWeaponEvent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
//
// protected:
// 	virtual void Notify(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
//
// protected:
// 	UPROPERTY(EditAnywhere)
// 	EWeaponHandType WeaponHandType = EWeaponHandType::LeftHand;
// 	
// 	UPROPERTY(EditAnywhere)
// 	FGameplayEventData EventData;
// };
