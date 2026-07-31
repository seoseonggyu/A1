# A1 — Unreal Engine 5.8 C++ 프로젝트

**필요한 경우 주석·문서·응답은 한국어로 작성한다** 식별자는 영어 UpperCamelCase.

## 빌드

- 헤더 추가/이동 시 **프로젝트 파일 재생성 → 빌드** 순서.
- 사용자가 직접 빌드를 하기 때문에. Claude Code는 빌드를 하라고 제안한다.

## 디렉터리

```
Source/A1/                 게임 모듈 (A1_API, 클래스 접두사 A1)
  AbilitySystem/ Player/ System/ Weapon/ Cosmetic/ Actors/ UI/ DataAsset/
Plugins/
  CommonGame/              핵심 프레임워크: Experience, ActorExtension, GAS, Inventory,
                           Equipment, QuickBar, Camera, Input, AssetManager
  CommonUIExtension/       PrimaryGameLayout(레이어), UIExtensionPoint, MVVM ViewModel
  CommonCoroutine/         C++20 코루틴 (TCoroTask<T>, Coro::Latent / Async / GAS)
  ModularGameplayActors/   AModularCharacter 등 GFCM 등록 베이스
  GameFeatures/Ark/        콘텐츠 전용 GameFeature (ExplicitlyLoaded, 기본 Registered)
Config/                    DefaultEngine/Game/Input.ini
```

## 코딩 규칙

- 접두사: `U`(UObject) / `A`(Actor) / `F`(struct) / `E`(enum) / `I`(interface), 게임 모듈 클래스는 `A1` 접두사(`AA1Character`, `UA1VitalSet`).
- 포인터: 멤버는 `TObjectPtr<T>`, 약참조는 `TWeakObjectPtr<T>`, 지연 로드는 `TSoftObjectPtr/TSoftClassPtr`.
- 로그: 파일마다 전용 카테고리.
  헤더 `DECLARE_LOG_CATEGORY_EXTERN(<클래스명>Log, Log, All);` / cpp `DEFINE_LOG_CATEGORY(<클래스명>Log);`
- 단정: 불변식은 `check()`. 복구 가능한 상황은 `if (!X) { UE_LOG(...); return; }`.
- 주석: 클래스/구조체 위에는 역할·동작 방식을 설명하는 `/** */` 블록, 멤버는 한 줄 `/** */`. 기존 파일의 `//---` 섹션 구분선 스타일을 따른다.
- 함수명 접미사로 실행 권한을 표기한다 (**반드시 지킬 것**):

| 접미사 | 의미 | 예 |
|---|---|---|
| `...Auth` | 서버(Authority) 전용. `UFUNCTION(BlueprintAuthorityOnly)` 동반 | `RemoveItemAuth` |
| `...Server` | 클라 → 서버 RPC. `UFUNCTION(Server, Reliable)` | `ModifyTagStatServer` |
| `...Local` | 호출한 쪽에서만 실행(비복제) | `SetTagStatValueLocal` |
| `...Coroutine` | `TCoroTask<T>` 반환 | `AddItemAuthCoroutine` |

## 작업 시 주의

- 새 UCLASS/USTRUCT 추가 시 `.generated.h`를 **마지막 include**로 넣고, 헤더는 전방 선언 우선.
- `.uasset`(Blueprint, DataAsset)은 바이너리라 편집 불가. C++ 변경이 BP 참조를 깨뜨릴 수 있으면 **먼저 알리고** 진행한다.
  (`Plugins/GameFeatures/Ark/Content` 및 `/Game` 하위)
- 복제 프로퍼티 추가 시 `GetLifetimeReplicatedProps` 갱신 필수.

## 작업 후

- 코드가 생성, 삭제, 변경이 되었으면 바뀐 부분이 어디인지를 알려준다.