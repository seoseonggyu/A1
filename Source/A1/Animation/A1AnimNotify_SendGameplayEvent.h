// #pragma once
//
// #include "GameplayTagContainer.h"
// #include "Abilities/GameplayAbilityTypes.h"
// #include "A1AnimNotify_SendGameplayEvent.generated.h"
//
// UCLASS(meta=(DisplayName="Send Gameplay Event"))
// class UA1AnimNotify_SendGameplayEvent : public UAnimNotify
// {
// 	GENERATED_BODY()
// 	
// public:
// 	UA1AnimNotify_SendGameplayEvent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
//
// protected:
// 	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
//
// protected:
// 	UPROPERTY(EditAnywhere)
// 	FGameplayTag EventTag;
//
// 	UPROPERTY(EditAnywhere)
// 	FGameplayEventData EventData;
// };
