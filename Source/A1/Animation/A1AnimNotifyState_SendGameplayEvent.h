#pragma once

#include "A1AnimNotify_SendGameplayEvent.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "A1AnimNotifyState_SendGameplayEvent.generated.h"

UCLASS(meta=(DisplayName="Send Gameplay Event State"))
class UA1AnimNotifyState_SendGameplayEvent : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	UA1AnimNotifyState_SendGameplayEvent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
protected:
	UPROPERTY(EditAnywhere)
	FGameplayTag BeginEventTag;

	UPROPERTY(EditAnywhere)
	FGameplayTag TickEventTag;
	
	UPROPERTY(EditAnywhere)
	FGameplayTag EndEventTag;

	UPROPERTY(EditAnywhere)
	FGameplayEventData EventData;
};
