#pragma once

#include "Abilities/Tasks/AbilityTask.h"
#include "A1AbilityTask_WaitForTick.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTickTaskDelegate, float, DeltaTime);

/**
 * UA1AbilityTask_WaitForTick
 *
 * 어빌리티가 활성화되어 있는 동안 매 프레임 OnTick을 브로드캐스트하는 지속형 태스크.
 * 태스크가 생성된 쪽(서버·소유 클라 각각)에서만 틱이 돌며, EndTask/EndAbility 시 자동 정리된다.
 * 지속적인 조건 감시(예: 스프린트 유지 여부)에 사용한다.
 */
UCLASS()
class UA1AbilityTask_WaitForTick : public UAbilityTask
{
	GENERATED_BODY()

public:
	UA1AbilityTask_WaitForTick(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta=(HidePin="OwningAbility", DefaultToSelf="OwningAbility", BlueprintInternalUseOnly="true"))
	static UA1AbilityTask_WaitForTick* WaitForTick(UGameplayAbility* OwningAbility);

protected:
	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;

public:
	UPROPERTY(BlueprintAssignable)
	FOnTickTaskDelegate OnTick;
};
