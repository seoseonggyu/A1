---
name: a1-gamefeature-extension
description: A1 프로젝트의 Experience, GameFeature, GameFeatureAction, ActorExtension(Condition/Execute), ActorExtensionWorldSubsystem, 모듈러 컴포넌트 주입을 다룰 때 반드시 사용한다. "게임 피처", "익스피리언스", "모드별로 다른 기능", "Pawn에 기능 붙이기", "액터 확장", "GFCM", "Ark 플러그인" 같은 요청이면 이 스킬을 확인할 것.
---

# Experience / GameFeature / ActorExtension (A1)

## 로딩 흐름

```
ACommonWorldSettings (맵별 기본 Experience)
      ↓
ACommonGameModeBase  ── Experience 결정, 로드 완료까지 Pawn 스폰 대기[a1-coroutine](../a1-coroutine)
      ↓
UExperienceManagerComponent (GameState)
      ↓  GameFeature 플러그인 활성화
UGameFeatureAction_*  ── Ability / Attribute / Widget / ActorExtension / PreloadAssets 주입
```

`UExperienceDefinition` : `DefaultPawnClass` + `GameFeaturesToEnable`(FPrimaryAssetId 배열).
새 Experience/GameFeature를 추가하면 `Config/DefaultGame.ini`의 `PrimaryAssetTypesToScan`에도 등록해야 한다.

## Component vs Extension

| | Component | ActorExtension |
|---|---|---|
| 상태 보관·복제 | O | X (런타임 상태만) |
| Actor 구조 변경 | 있음 | 없음 |
| 조건부 활성화 | 직접 구현 | Condition으로 선언적 처리 |

→ **데이터/복제가 필요하면 Component, 행위만 주입이면 Extension.**

## ActorExtension 확장

`FActorExtension` = `TArray<TInstancedStruct<FExtensionCondition>> Conditions` (AND) + `TArray<TInstancedStruct<FExtensionExecute>> Executes`.
`UActorExtensionWorldSubsystem`이 Tick으로 Unchecked → Register → Complete 상태를 관리한다.

### 새 Condition

```cpp
// Plugins/CommonGame/.../Extension/Condition/ExtensionCondition_<이름>.h
USTRUCT(BlueprintType)
struct COMMONGAME_API FExtensionCondition_<이름> : public FExtensionCondition
{
    GENERATED_BODY()
    virtual bool IsSatisfied(AActor* Owner) const override;
};
```

기존: `_NetworkReady`(Controller+PlayerState 준비), `_HasInputComponent`.

### 새 Execute

```cpp
// Plugins/CommonGame/.../Extension/Execute/ExtensionExecute_<이름>.h
USTRUCT(BlueprintType)
struct COMMONGAME_API FExtensionExecute_<이름> : public FExtensionExecute
{
    GENERATED_BODY()
    virtual void OnActivate(AActor* Owner) const override;
    virtual void OnDeactivate(AActor* Owner) const override;   // 반드시 대칭으로 정리
};
```

기존: `_InitAbilitySystem`, `_BindInput_TopDown`, `_SetCameraMode`.

- 두 베이스 모두 `const` 함수이므로 런타임 상태 멤버는 `mutable`로 선언한다.
- Pawn 전용 로직이면 `Cast<APawn>` 실패 시 조용히 반환한다(기존 구현 패턴).

## 새 GameFeatureAction

- 월드/네트워크 필터링이 필요하면 `UGameFeatureAction_WorldNetworkBase`를 상속하고 `bClientAction` / `bServerAction`을 활용한다.
- 부여한 핸들은 Context별 구조체(`FAbilityGrantedHandles` 패턴)에 모아 두고 `OnGameFeatureDeactivating`에서 전부 해제한다. **해제 누락은 이 시스템에서 가장 흔한 버그다.**
- `TSoftObjectPtr`/`TSoftClassPtr` 프로퍼티에는 `meta = (AssetBundles = "Client")` 등 번들 태그를 붙인다.

## Ark 플러그인

`Plugins/GameFeatures/Ark` — 콘텐츠 전용(`ExplicitlyLoaded: true`, 초기 상태 `Registered`).
Ability/Equipment/UI/Input/Camera/Character BP가 여기 있다. **`.uasset`은 편집 불가이므로 C++ 시그니처를 바꿀 때 BP 참조가 깨질 수 있음을 먼저 알린다.**
