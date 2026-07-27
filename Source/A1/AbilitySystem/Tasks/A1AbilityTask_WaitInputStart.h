#pragma once

#include "Abilities/Tasks/AbilityTask.h"
#include "A1AbilityTask_WaitInputStart.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInputStartDelegate);


/**
 * UA1AbilityTask_WaitInputStart
 *
 * GameCustom1 신호가 들어올 때까지 대기하다가, 들어오면 OnStart를 브로드캐스트하고 스스로 종료되는 1회용 태스크.
 * 이미 활성화되어 있는 어빌리티에 "입력이 또 들어왔다"를 전달하기 위한 통로 (콤보 입력 등).
 * 서버와 소유 클라이언트 양쪽에서 각각 하나씩 생성되며, 각자의 ASC에 델리게이트를 등록한다.
 * 클라가 신호를 쏘면 로컬에서 즉시 반응(예측)하고 서버로 RPC를 보내며, 서버는 그 RPC를 받고 뒤늦게 반응한다.
 * 서버가 이 흐름을 도는 이유는 서버 자신의 상태를 갱신해 콤보 진행 여부를 권위적으로 판정하기 위함이다.
 *
 * 1회용이므로 연속 입력을 받으려면 OnStart 콜백에서 태스크를 다시 생성해야 한다.
 * 입력이 눌린 "순간"에만 반응하며, 눌려있는 동안이나 떼는 순간은 감지하지 않는다.
 */
UCLASS()
class UA1AbilityTask_WaitInputStart : public UAbilityTask
{
	GENERATED_BODY()

public:

	UA1AbilityTask_WaitInputStart(const FObjectInitializer& ObjectInitializer);
	
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta=(HidePin="OwningAbility", DefaultToSelf="OwningAbility", BlueprintInternalUseOnly="true"))
	static UA1AbilityTask_WaitInputStart* WaitInputStart(UGameplayAbility* OwningAbility);
	
public:
	
	/**
	 * 태스크 시작 지점. ASC의 GameCustom1 채널에 OnStartCallback을 등록한다.
	 * 등록 외에 능동적으로 하는 일은 없으며, 누군가 신호를 쏴줄 때까지 아무 코드도 돌지 않는다.
	 *
	 * 서버(IsForRemoteClient)에 한해, 등록 이전에 클라 신호가 먼저 도착해 있었는지 1회 확인한다.
	 * 도착해 있었다면 그 자리에서 즉시 콜백을 실행하고, 아니면 대기 상태로 진입한다.
	 */
	
	virtual void Activate() override;
	
public:
	
	/**
	 * 신호 수신 시 실행. 순서가 중요하다.
	 *
	 * 1. 델리게이트 해제  - 아래에서 생성될 새 태스크의 등록과 충돌하지 않도록 먼저 제거
	 * 2. 예측 윈도우 진입 - 이 스코프 안의 예측성 변경을 하나의 PredictionKey로 묶는다.
	 *                       클라가 미리 한 일과 서버가 실제로 한 일을 같은 사건으로 식별하기 위한 것이며,
	 *                       예측 실패 시 롤백과 예측 성공 시 중복 적용 방지에 모두 쓰인다.
	 * 3. 클라 : 서버로 RPC 전송. 서버 측 대기 중인 태스크를 깨운다.
	 *    서버 : 신호 소비(bTriggered = false).
	 *           우편함 키가 어빌리티 활성화 단위로 유지되므로, 소비하지 않으면
	 *           아래에서 생성될 새 태스크가 방금 처리한 신호를 다시 보고 즉시 재발동한다.
	 * 4. 어빌리티가 유효한 경우에 한해 OnStart 브로드캐스트
	 * 5. 브로드캐스트 여부와 무관하게 EndTask 호출
	 */
	UFUNCTION()
	void OnStartCallback();

public:
	UPROPERTY(BlueprintAssignable)
	FInputStartDelegate OnStart;
	
protected:
	FDelegateHandle DelegateHandle;
};
