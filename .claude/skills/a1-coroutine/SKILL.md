---
name: a1-coroutine
description: A1 프로젝트에서 비동기·지연 로직을 작성할 때 반드시 사용한다. TCoroTask, co_await, Coro::Latent/Async/GAS, 에셋 비동기 로딩, 딜레이, 대기, 델리게이트 대기, 프레임 대기, GameFeature 로딩, 스레드 전환 관련 요청이면 "코루틴"이라는 말이 없어도 이 스킬을 확인할 것. 타이머·Latent Action·AbilityTask를 새로 만들려 할 때도 먼저 이 스킬을 볼 것.
---

# 코루틴 (CommonCoroutine)

이 프로젝트는 타이머/AbilityTask 대신 **C++20 코루틴을 기본 비동기 수단**으로 쓴다. 새로 Latent Action이나 Timer 체인을 만들기 전에 코루틴으로 표현 가능한지 먼저 검토할 것.

## 사용법

```cpp
#include "Coro.h"     // 이 헤더 하나만 include (내부 include 순서가 중요함)

TCoroTask<void> UMyClass::DoSomethingCoroutine()
{
    co_await Coro::Latent::Seconds(this, 1.0);
    co_return;
}

TCoroTask<UItemInstance*> UMyClass::MakeItemCoroutine()
{
    co_return NewItem;
}
```

- 함수명 접미사는 `...Coroutine`. 서버 전용이면 `...AuthCoroutine`.
- 모든 Awaiter의 첫 인자는 `UObject* Owner` — Owner가 파괴되면 코루틴이 안전하게 중단된다. `this`를 넘기는 것이 기본.
- 반환된 `TCoroTask`를 버리지 말고 멤버로 보관해야 하는 경우가 있다 (`UEquipmentComponent::PendingInitTasks` 참고).

## Awaiter 목록

| 네임스페이스 | 함수 |
|---|---|
| `Coro::Latent` | `Seconds(Owner, Duration)`, `Frames(Owner, Count)`, `NextTick(Owner)`, `Until(Owner, Predicate)` |
| `Coro::Async` | `LoadPrimaryAssets(Owner, AssetIds, Bundles)`, `LoadGameFeature(Owner, PluginURL)`, `WhenAll(Owner, Tasks)`, `WhenAny(Owner, Tasks)`, `WaitForDelegate(Owner, Delegate)`, `MoveToThread(Owner, Thread)`, `MoveToGameThread(Owner)`, `MoveToTask(Owner)` |
| `Coro::GAS` | `WaitForTargetData(Owner, ASC, SpecHandle, PredictionKey)` |
| CommonGame 확장 | `Coro::LoadBundle(Asset)` — `UCommonPrimaryDataAsset` 번들 로딩 |

## 규칙

- `MoveToThread`로 게임 스레드를 벗어난 뒤 UObject를 만지려면 반드시 `MoveToGameThread(this)`로 돌아온다.
- 코루틴 안에서 `co_await` 이후에는 Owner/포인터의 유효성을 다시 확인한다(`IsValid`).
- 여러 비동기 작업이 있으면 순차 `co_await` 대신 `WhenAll`을 쓴다.
- 블루프린트에 노출할 수 없다. BP 노출이 필요하면 코루틴을 내부에 두고 델리게이트/`UFUNCTION` 래퍼를 제공한다.
