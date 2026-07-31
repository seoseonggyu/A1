#include "A1AbilityTask_WaitForTick.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1AbilityTask_WaitForTick)

UA1AbilityTask_WaitForTick::UA1AbilityTask_WaitForTick(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = true;
}

UA1AbilityTask_WaitForTick* UA1AbilityTask_WaitForTick::WaitForTick(UGameplayAbility* OwningAbility)
{
	UA1AbilityTask_WaitForTick* Task = NewAbilityTask<UA1AbilityTask_WaitForTick>(OwningAbility);
	return Task;
}

void UA1AbilityTask_WaitForTick::Activate()
{
	Super::Activate();
}

void UA1AbilityTask_WaitForTick::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnTick.Broadcast(DeltaTime);
	}
}
