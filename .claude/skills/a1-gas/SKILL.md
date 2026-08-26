---
name: a1-gas
description: A1 프로젝트에서 GameplayAbility, AttributeSet, GameplayEffect, 네이티브 GameplayTag, 입력-어빌리티 연동을 추가하거나 수정할 때 반드시 사용한다. "어빌리티", "스킬", "공격", "속성", "스탯", "HP/MP/스태미나", "GAS", "ASC", "태그", "인풋 바인딩" 같은 말이 나오면 명시적으로 GAS를 언급하지 않아도 이 스킬을 확인할 것.
---

# GAS (A1)

## 구성

- ASC: `UCommonAbilitySystemComponent` — **PlayerState**에 부착. `ACommonCharacter::GetAbilitySystemComponent()`는 PlayerState의 ASC를 반환.
- Ability 베이스: `UCommonGameplayAbility` (ActivationPolicy / ActivationGroup, 입력 태그 기반 활성화)
- 게임 측 계층: `UA1Ability_Equipment` → `UA1Ability_MeleeWeaponAttack`/`_MeleeWeaponComboAttack`, `UA1Ability_Consume` → `UA1Ability_DrinkPotion`. 도메인별로 `AbilitySystem/Interaction`(Interact, Interact_Scan/Door/Pickup/Player), `Movement`(Sprint_Check/Active), `Skill`(GroundBreaker, WhirlwindSlash), `Weapon` 하위 폴더에 분리.
- 입력 없이 이벤트로만 발동되는 Ability도 많다: `ActivationPolicy=Manual` + `AbilityTriggers=GameplayEvent.*`. 예) `UA1Ability_Death`(`GameplayEvent.Death`), `UA1Ability_DropItem`(`GameplayEvent.DropItem`, UI가 `HandleGameplayEvent` 직접 호출), `UA1Ability_Interact_Player`(`GameplayEvent.Interact.Player`, `NetExecutionPolicy=ServerInitiated`로 서버 판단을 소유 클라에 복제).
- AttributeSet: `UCommonAttributeSet` → `UA1VitalSet` (Health / Mana / Stamina)
- 초기화: `FExtensionExecute_InitAbilitySystem`이 `InitAbilityActorInfo` + `OnPostProcessInput` → `ProcessAbilityInput` 바인딩 수행. **직접 초기화 코드를 새로 짜지 말 것.**

## 새 Ability 추가

1. 위치: `Source/A1/AbilitySystem/A1Ability_<이름>.h/.cpp`
2. `UA1Ability_Equipment`(장비 연동) 또는 `UCommonGameplayAbility`(범용)을 상속.
3. 전용 로그 카테고리 선언 + 클래스 doc 주석(한국어).
4. 오버라이드 순서: `CanActivateAbility` → `ActivateAbility` → `EndAbility`.
5. `ActivateAbility` 마지막에는 반드시 `EndAbility(...)` 경로가 보장되어야 한다.
6. 대기·연출은 AbilityTask 대신 코루틴을 우선 검토 (`a1-coroutine`).

## 입력 연동

1. `A1GameplayTags.h`에 `A1_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_<이름>);`
2. `A1GameplayTags.cpp`에 `UE_DEFINE_GAMEPLAY_TAG(Input_Ability_<이름>, "Input.Ability.<이름>");`
3. 부여 시 `FCommonAbilityEntry.InputTag`에 지정 → `DynamicSpecSourceTags`로 들어가 ASC가 매칭 활성화.
4. IA 에셋 ↔ 태그 매핑은 `FInputActionAndTag` 배열(BP/DataAsset). `.uasset` 편집은 사용자 몫이므로 필요한 항목을 안내만 한다.

## Attribute 추가 (`UA1VitalSet`)

```cpp
// 헤더
ATTRIBUTE_ACCESSORS(UA1VitalSet, <Name>);   // CommonAttributeSet 매크로

UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_<Name>, Category = "Vital")
FGameplayAttributeData <Name>;

UFUNCTION() void OnRep_<Name>(const FGameplayAttributeData& Old);
```

- `GetLifetimeReplicatedProps`에 `DOREPLIFETIME_CONDITION_NOTIFY(..., COND_None, REPNOTIFY_Always)` 추가.
- `OnRep_`에서 `GAMEPLAYATTRIBUTE_REPNOTIFY` 호출.
- 클램프는 `PreAttributeChange` / `PostGameplayEffectExecute`에서 처리.
- UI 노출이 필요하면 `UCharacterViewModel`에도 프로퍼티·델리게이트를 추가한다 (`a1-ui-mvvm`).

## 부여 방식

- Experience/GameFeature 단위 → `UGameFeatureAction_AddAbilities` / `UGameFeatureAction_AddAttributes`
- 장비 장착 단위 → `FEquipmentFragment_Ability` (`a1-item-equipment`)
- **C++에서 `GiveAbility`를 직접 호출하는 새 경로는 만들지 않는다.**

## 체크리스트

- [ ] 서버/클라 실행 위치를 확인했는가 (`HasAuthority`, 예측 필요 여부)
- [ ] 로그 카테고리 선언·정의 쌍이 맞는가
- [ ] 태그 문자열이 `Input.Ability.*` / `Equipment.Slot.*` / `QuickBar.Slot.*` 규칙을 따르는가
- [ ] 새 Attribute의 복제 설정을 빠뜨리지 않았는가
- [ ] Ability 종료 후에도 남아야 하는 상태 태그(예: `Status.Death`)를 `SetLooseGameplayTagCount`로 직접 설정했다면, Iris 하에서는 `EGameplayTagReplicationState::TagAndCountToAll`을 지정했는가 (기본값 `None`은 소유 클라에만 보이고 다른 클라에는 복제되지 않는다)
