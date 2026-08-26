---
name: a1-ui-mvvm
description: A1 프로젝트의 UI를 다룰 때 반드시 사용한다. ViewModel(MVVM), FieldNotify, PrimaryGameLayout 레이어, UIExtensionPoint, CommonActivatableWidget, HUD, 위젯 푸시/팝, UIPolicy, 인벤토리 창 같은 화면 작업이면 "MVVM"이라는 말이 없어도 이 스킬을 확인할 것.
---

# UI / MVVM (A1)

## 구성

```
UCommonUIManagerSubsystem (GameInstance)
      └─ UCommonUIPolicy ── LocalPlayer마다 UCommonPrimaryGameLayout 생성
                                  └─ 레이어(GameplayTag) 별 위젯 스택
                                        └─ UUIExtensionPointWidget ← UUIExtensionSubsystem
```

- 기본 레이어 태그: `UI.Layer.Game`(HUD), `UI.Layer.GameMenu`, 그 외 `CommonUIExtensionTags` 참조.
- 위젯 베이스: `UCommonExtensionActivatableWidget`(전체 화면/메뉴), `UCommonExtensionUserWidget`(부품), 게임 측 `UA1ActivatableWidget`.
- 입력 모드는 `ECommonWidgetInputMode`(Default / GameAndMenu / Game / Menu)로 지정.
- 헬퍼: `UCommonUIExtensions`(레이아웃 접근, 위젯 푸시 등 static 함수).
- 데디케이티드 서버에서는 UI 서브시스템이 생성되지 않는다 — UI 코드는 클라이언트 전용으로 가정.

## ViewModel 추가

`Source/A1/UI/<도메인>/<이름>ViewModel.h/.cpp`, `UCommonViewModelBase` 상속.

```cpp
UCLASS(BlueprintType)
class A1_API U<이름>ViewModel : public UCommonViewModelBase
{
    GENERATED_BODY()
public:
    virtual void InitializeViewModel(UObject* SourceObject) override;
    virtual void UninitializeViewModel() override;

    UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "<이름> ViewModel|<그룹>")
    float Value = 0.f;

    static const FName ViewModelName;   // MVVM 바인딩 식별자
};
```

- 값 갱신은 반드시 `UE_MVVM_SET_PROPERTY_VALUE(Prop, NewValue)` 로 한다(직접 대입 시 알림이 안 감).
- 소스 오브젝트는 `TWeakObjectPtr`로 보관.
- `InitializeViewModel`에서 건 델리게이트는 `UninitializeViewModel`에서 **전부 해제**한다.
- GAS 스탯을 노출할 때는 `UCharacterViewModel`의 `UPDATE_ATTRIBUTE_GROUP` 매크로 패턴(Current/Max/Percent 3종 세트)을 따른다.

## UIExtensionPoint

- 수신 측(HUD 위젯): `RegisterExtensionPoint(Tag, Match, ...)`
- 공급 측(GameFeature/장비): `UGameFeatureAction_AddWidgets` 또는 `FEquipmentFragment_UIExtension`
- 매칭 규칙 `EUIExtensionPointMatch`: `ExactMatch` / `PartialMatch`(하위 태그 포함)
- 태그 규칙: `UI.ExtensionPoint.*`

## 위젯 구현 패턴: ViewModel vs 직접 바인딩

- 단순 값 표시(스탯 등)는 **ViewModel(FieldNotify)** 패턴을 쓴다 (`UCharacterViewModel`).
- 인벤토리/장비/루팅/드래그드롭처럼 상태가 복잡한 위젯(`UA1InventoryWidget`, `UA1EquipmentWidget`, `UA1LootWidget` 등, `Source/A1/UI/Inventory|Equipment|Loot`)은 ViewModel을 거치지 않고 **컴포넌트 델리게이트에 직접 바인딩**하는 `UCommonExtensionActivatableWidget` 파생 C++ 클래스로 만든다 (예: `OnInventoryGridChanged` 구독).
- 다른 Pawn을 대상으로 같은 위젯을 재사용하는 패턴(시체 루팅 창)은 `SetTargetPawnOverride(TargetPawn, bReadOnly)` 같은 함수로 대상 교체를 지원한다. `InventoryComponent`/`EquipmentComponent`가 조건 없이 전체 복제되므로 다른 플레이어의 컴포넌트를 그대로 바인딩해도 실시간으로 갱신된다.

## 규칙

- 위젯 배치·바인딩은 BP(`.uasset`) 쪽 작업이다. C++에서는 ViewModel·ExtensionPoint·레이어 API까지만 담당하고, BP에서 해야 할 일은 **안내만** 한다.
- `UI.Layer.*` / `UI.ExtensionPoint.*` 태그는 네이티브 태그로 선언해서 문자열 오타를 막는다.
- 새 UIPolicy 클래스를 만들면 `Config/DefaultGame.ini`의 `DefaultUIPolicyClass`도 함께 확인한다.
