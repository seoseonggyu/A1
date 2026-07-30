---
name: a1-item-equipment
description: A1 프로젝트의 인벤토리·아이템·장비·퀵바 시스템을 다룰 때 반드시 사용한다. ItemDefinition/ItemInstance/ItemFragment, EquipmentDefinition/EquipmentInstance/EquipmentFragment, InventoryComponent, EquipmentComponent, QuickBarComponent, 무기(WeaponInstance), 슬롯, 아이템 스탯(TagStat) 관련 요청이면 "프래그먼트"라는 말이 없어도 이 스킬을 확인할 것.
---

# 아이템 / 장비 (A1)

## 데이터 흐름

```
UItemDefinition (불변 DataAsset, Fragment 템플릿 배열)
      └─> UItemInstance (런타임, Fragment 복사본 + NetState)
              └─ FItemFragment_Equipment ─> UEquipmentDefinition
                                                 └─> UEquipmentInstance (+ 스폰 Actor)
                                                        └─ UWeaponInstance / UMeleeWeaponInstance
```

- `UInventoryComponent` : **Controller** 부착. Owner에게만 복제. Experience 로드 후 초기 아이템 지급.
- `UEquipmentComponent` : **Pawn** 부착. 슬롯 태그 → `UEquipmentInstance` 맵 관리.
- `UQuickBarComponent` : 슬롯 태그별 아이템 배열 + 활성 슬롯.

## 주요 API

```cpp
// 인벤토리 (서버)
TCoroTask<UItemInstance*> AddItemAuthCoroutine(const UItemDefinition*, int32 Count = 1);
bool RemoveItemAuth(UItemInstance*);
bool ModifyStackCountAuth(UItemInstance*, int32 NewCount);

// 조회
UItemInstance* FindItemById(int32) const;
template<typename T> UItemInstance* FindItemWithFragment() const;

// 장비 (서버)
TCoroTask<UEquipmentInstance*> EquipItemAuthCoroutine(UItemInstance*);
bool UnequipItemAuth(UEquipmentInstance*);
UEquipmentInstance* GetEquipmentInSlot(FGameplayTag SlotTag) const;

// 퀵바 (서버)
bool AddItemToSlotAuth(UItemInstance*);
void SetActiveSlotAuth(FGameplayTag SlotTag, int32 Index = 0);
UItemInstance* GetActiveItem() const;

// 컴포넌트 획득
UInventoryComponent::FindInventoryComponent(Pawn 또는 Controller);
UEquipmentComponent::FindEquipmentComponent(Pawn);
```

## Fragment 추가

기능은 상속이 아니라 **Fragment 조합**으로 넣는다. 새 클래스를 파생시키기 전에 Fragment로 표현 가능한지 먼저 검토할 것.

### ItemFragment

`Plugins/CommonGame/.../Inventory/Fragment/ItemFragment_<이름>.h`

```cpp
USTRUCT(BlueprintType)
struct COMMONGAME_API FItemFragment_<이름> : public FItemFragment
{
    GENERATED_BODY()

    virtual void OnCreated(UItemInstance* Owner) override;
    virtual void OnChanged(UItemInstance* Owner) override;
};
```

### EquipmentFragment

`Plugins/CommonGame/.../Equipment/Fragment/EquipmentFragment_<이름>.h`

```cpp
USTRUCT(BlueprintType)
struct COMMONGAME_API FEquipmentFragment_<이름> : public FEquipmentFragment
{
    GENERATED_BODY()

    virtual void OnEquipped(UEquipmentInstance* Instance) override;
    virtual void OnUnequipped(UEquipmentInstance* Instance) override;

    // 런타임 핸들은 mutable로 추적하고 OnUnequipped에서 반드시 정리
};
```

기존 예시: `_Ability`(어빌리티 부여), `_GameplayEffect`, `_AnimationData`, `_UIExtension`(CommonUIExtension 모듈).

## 아이템 스탯 (TagStat)

```cpp
Instance->ModifyTagStatAuth(StatTag)->Value = 10.f;   // 서버, 복제됨
Instance->GetTagStatValue(StatTag, OutValue);          // 조회
Instance->SetTagStatValueLocal(StatTag, 5.f);          // 로컬 예측
Instance->ModifyTagStatServer(...);                    // 클라 → 서버 RPC
```

## 규칙

- **상태 변경은 전부 서버(`...Auth`)에서.** 클라이언트에서는 조회·표시만 한다.
- `UItemInstance`는 클라이언트에서 `PostReplicatedAdd` 시 로컬 생성된다. 복제되는 것은 `Definition`, `ItemId`, `NetState`뿐 — Fragment 자체는 복제되지 않는다.
- Fragment 배열 인덱스는 **최대 16개**(NetState의 `FragmentIndex`가 uint8/비트 제한). 초과 설계 금지.
- 복제 상태가 필요한 Fragment는 `FItemFragmentNetState` 파생이 필요하다 → `a1-replication` 참조.
- 슬롯 태그는 `Equipment.Slot.*`, `QuickBar.Slot.*`. `EEquipmentSlotType` / `EArmorType`(`A1Define.h`)와 대응 관계를 깨지 말 것.
- 장착 초기화는 비동기(에셋 로딩)라 `PendingInitTasks`로 코루틴 관리 중. 동기 가정 코드를 넣지 않는다.
