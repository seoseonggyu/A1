#include "A1AbilityTask_WaitInputStart.h"

#include "AbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1AbilityTask_WaitInputStart)

UA1AbilityTask_WaitInputStart::UA1AbilityTask_WaitInputStart(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UA1AbilityTask_WaitInputStart* UA1AbilityTask_WaitInputStart::WaitInputStart(UGameplayAbility* OwningAbility)
{
	UA1AbilityTask_WaitInputStart* Task = NewAbilityTask<UA1AbilityTask_WaitInputStart>(OwningAbility);
	return Task;
}

void UA1AbilityTask_WaitInputStart::Activate()
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (ASC && Ability)
	{
		// "GameCustom1 이벤트가 오면 나를 불러줘." 라고 등록
		// 쉽게 말해 ASC -> GameCustom1 -> OnStartCallback
		DelegateHandle = ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::GameCustom1, GetAbilitySpecHandle(), GetActivationPredictionKey()).AddUObject(this, &UA1AbilityTask_WaitInputStart::OnStartCallback);

		// IsForRemoteClient
		// - 이 태스크가 서버에서, 원격 클라이언트를 대신해 돌고 있는가?를 알려주는 함수
		// - 즉 권한(서버)이면서 로컬 제어가 아닐 때 true
		// 서버 + 플레이어가 조종하는 폰 → true
		// 서버 + AI가 조종하는 폰 → false (서버가 곧 로컬 컨트롤러라서)
		// 클라이언트 쪽 → 항상 false (권한이 없으므로)
		if (IsForRemoteClient())
		{
			// CallReplicatedEventDelegateIfSet
			// - 이미 이벤트가 도착했어?
			// YES -> 바로 실행
			// No -> Wait, false가 나오면 SetWaitingOnRemotePlayerData 함수가 실행되면서 Task는 Sleep 상태가 된다
			// Sleep 상태를 깨우는 주체는 Local_Client.
			// 공격 버튼 -> ServerSetReplicatedEvent -> Server ASC -> GameCustom1 발생 -> Delegate 호출 -> OnStartCallback
			if (ASC->CallReplicatedEventDelegateIfSet(EAbilityGenericReplicatedEvent::GameCustom1, GetAbilitySpecHandle(), GetActivationPredictionKey()) == false)
			{
				SetWaitingOnRemotePlayerData();
			}
		}
	}
}

void UA1AbilityTask_WaitInputStart::OnStartCallback()
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (Ability == nullptr || ASC == nullptr)
		return;

	// Activate에서 걸어놓은 Delegate 제거 
	ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::GameCustom1, GetAbilitySpecHandle(), GetActivationPredictionKey()).Remove(DelegateHandle);

	// Prediction을 시작하는 객체
	// 이 안에서 발생하는 GameplayEffect나 GameplayEvent는 PredictionKey를 공유
	FScopedPredictionWindow ScopedPrediction(ASC, IsPredictingClient());

	// IsPredictingClient
	// - 클라이언트라면 ServerSetReplicatedEvent 호출
	// - 서버라면 ConsumeGenericReplicatedEvent 호출
	if (IsPredictingClient())
	{
		ASC->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GameCustom1, GetAbilitySpecHandle(), GetActivationPredictionKey(), ASC->ScopedPredictionKey);
	}
	else
	{
		ASC->ConsumeGenericReplicatedEvent(EAbilityGenericReplicatedEvent::GameCustom1, GetAbilitySpecHandle(), GetActivationPredictionKey());
	}

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnStart.Broadcast();
	}
	EndTask();
}
