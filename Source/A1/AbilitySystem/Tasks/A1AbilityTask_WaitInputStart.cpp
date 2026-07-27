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
		// 서버와 클라이언트 양쪽에서 똑같이 실행
		DelegateHandle = ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::GameCustom1, GetAbilitySpecHandle(), GetActivationPredictionKey()).AddUObject(this, &UA1AbilityTask_WaitInputStart::OnStartCallback);

		// IsForRemoteClient
		// - 이 태스크가 서버에서, 원격 클라이언트를 대신해 돌고 있는가?를 알려주는 함수
		// - 즉 권한(서버)이면서 로컬 제어가 아닐 때 true
		// 서버 + 플레이어가 조종하는 폰 → true ==> 원격 플레이어의 입력을 기다리는 서버 쪽 코드
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
			// 즉, 네트워크 코드의 전형적인 방어
			// 클라의 RPC가 서버 태스크 생성보다 먼저 도착할 수 있다. 
			// (랙, 패킷 순서, 어빌리티 활성화 타이밍 차이) 그런 경우엔 bTriggered가 이미 true로 캐시되어 
			// 있으므로 CallReplicatedEventDelegateIfSet이 그 자리에서 콜백을 실행.
			// 아직 안 왔으면 SetWaitingOnRemotePlayerData()로 "이 어빌리티는 원격 데이터 대기 중"이라고 표시.
			// 이 플래그는 클라가 응답 없이 나가버렸을 때 GAS가 어빌리티를 정리/취소할지 판단하는 데 쓰이는 것.
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
	// 즉, 이 스코프 안에서 발생하는 모든 예측성 변경(GE 적용 등)에 하나의 PredictionKey를 묶어둠.
	// 클라에선 새 키를 만들고, 서버에선 클라가 보낸 키를 재사용해서 같은 행동이 같은 키로 묶이게 한다. 
	// 이게 있어야 서버 결과가 도착했을 때 클라가 예측분을 정확히 되돌릴 수 있다.
	FScopedPredictionWindow ScopedPrediction(ASC, IsPredictingClient());

	// IsPredictingClient
	// - 클라이언트라면 ServerSetReplicatedEvent 호출 => 클라에서 서버로 "나 이거 누름" RPC로 전달 => 서버의 대기 중인 태스크가 깨어남
	// - 서버라면 ConsumeGenericReplicatedEvent 호출 => 신호를 소비. bTriggered를 false로 되돌려서, 나중에 다른 재생성된 Task가 이 낡은 신호를 보고 오작동하는 방지
	if (IsPredictingClient())
	{
		ASC->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GameCustom1, GetAbilitySpecHandle(), GetActivationPredictionKey(), ASC->ScopedPredictionKey);
	}
	else
	{
		ASC->ConsumeGenericReplicatedEvent(EAbilityGenericReplicatedEvent::GameCustom1, GetAbilitySpecHandle(), GetActivationPredictionKey());
	}

	// 어빌리티가 그사이 취소/종료됐으면 브로드캐스트하지 않고 조용히 끝냄. (죽은 어빌리티의 콜백이 도는 크래시 방지)
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnStart.Broadcast();
	}
	EndTask();
}
