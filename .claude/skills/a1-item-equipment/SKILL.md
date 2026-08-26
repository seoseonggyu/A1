---
name: a1-item-equipment
description: A1 프로젝트의 인벤토리·아이템·장비·루팅 시스템을 다룰 때 반드시 사용한다. ItemDefinition/ItemInstance/ItemFragment, EquipmentDefinition/EquipmentInstance/EquipmentFragment, InventoryComponent(2D 그리드), EquipmentComponent(퀵바 슬롯 포함), 무기(WeaponInstance), 시체 루팅(Transfer*Server), 아이템 스탯(TagStat) 관련 요청이면 "프래그먼트"라는 말이 없어도 이 스킬을 확인할 것.
---

# 아이템 / 장비 (A1)

## 데이터 흐름

```
UItemDefinition (불변 DataAsset, SlotCount(2D 그리드 크기) + Fragment 템플릿 배열)
      └─> UItemInstance (런타임, Fragment 복사본 + NetState)
              └─ FItemFragment_Equipment ─> UEquipmentDefinition
                                                 └─> UEquipmentInstance (+ 스폰 Actor)
                                                        └─ UWeaponInstance / UMeleeWeaponInstance
```

- `UInventoryComponent` : **Pawn** 부착. 2D 그리드(`GridSize`, `OccupiedCells`, 아이템별 `SlotPosition` 앵커) 기반. `EquipmentComponent`와 동일하게 조건 없이 전체 복제(다른 플레이어도 열람 가능 — 사망한 캐릭터의 인벤토리를 다른 플레이어가 시체 루팅 UI로 열람). BeginPlay 시 초기 아이템 지급.
- `UEquipmentComponent` : **Pawn** 부착. 슬롯 태그(`Equipment.Slot.*`) → `UEquipmentInstance` 맵 관리, 조건 없이 전체 복제. **별도 QuickBar 컴포넌트는 없다** — 손에 드는 메인 장비는 `EquipmentSlots` 중 `bActive=true`인 하나(`MainEquippedItem`)로 표현되고, `QuickBar.Slot.*` 태그는 `SetActiveSlotServer`로 이 활성 슬롯을 전환하는 데만 쓰인다.

## 주요 API

```cpp
// 인벤토리 (서버)
TCoroTask<UItemInstance*> AddItemAuthCoroutine(const UItemDefinition*, int32 Count = 1);
bool RemoveItemAuth(UItemInstance*);
bool ModifyStackCountAuth(UItemInstance*, int32 NewCount);
bool MoveItemAuth(UItemInstance*, const FIntPoint& NewAnchor);   // 2D 그리드 이동
bool FindEmptySlot(const FIntPoint& Size, FIntPoint& OutSlotPos) const;

// 조회
UItemInstance* FindItemById(int32) const;
template<typename T> UItemInstance* FindItemWithFragment() const;

// 장비 (서버)
TCoroTask<UEquipmentInstance*> EquipItemAuthCoroutine(UItemInstance*);   // EquipmentComponent
void UnequipItemAuth(FGameplayTag SlotTag);                             // EquipmentComponent
UEquipmentInstance* GetEquipmentInSlot(FGameplayTag SlotTag) const;
UEquipmentInstance* GetActiveMainEquippedItem() const;

// 클라 → 서버 RPC (같은 캐릭터 내 이동/장착/해제)
InventoryComponent->MoveItemServer(ItemId, NewAnchor);
InventoryComponent->EquipFromInventoryServer(ItemId, SlotTag);
InventoryComponent->UnequipToInventoryServer(ItemId, FromSlotTag, Anchor);
EquipmentComponent->SetActiveSlotServer(SlotTag);

// 캐릭터 간 아이템 교환 (예: 시체 루팅). 반드시 자신이 소유한 InventoryComponent에서 호출하고
// Source/Dest 중 하나가 자기 자신이어야 한다.
InventoryComponent->TransferInventoryToInventoryServer(Source, ItemId, Dest, NewAnchor);
InventoryComponent->TransferInventoryToEquipmentServer(SourceInv, ItemId, DestEquip, SlotTag);
InventoryComponent->TransferEquipmentToInventoryServer(SourceEquip, SlotTag, DestInv, NewAnchor);
InventoryComponent->TransferEquipmentToEquipmentServer(SourceEquip, SrcSlot, DestEquip, DestSlot);

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

기존 예시: `_Ability`(어빌리티 부여), `_GameplayEffect`, `_AnimationData`, `_Consume`(소비 시 적용할 GE만 보관, 실행은 `UA1Ability_Consume`/`UA1Ability_DrinkPotion`), `_UIExtension`(CommonUIExtension 모듈).

## 아이템 스탯 (TagStat)

```cpp
Instance->ModifyTagStatAuth(StatTag)->Value = 10.f;             // 서버, 복제됨
Instance->GetTagStatValue(StatTag, OutValue);                   // 조회
Instance->SetTagStatValueLocal(StatTag, 5.f);                   // 로컬 예측
InventoryComponent->ModifyTagStatServer(ItemId, StatTag, 5.f);  // 클라 → 서버 RPC (InventoryComponent에 있음, ItemInstance 아님)
```

## 규칙

- **상태 변경은 전부 서버(`...Auth`)에서.** 클라이언트에서는 조회·표시만 한다.
- `UItemInstance`는 클라이언트에서 `PostReplicatedAdd` 시 로컬 생성된다. 복제되는 것은 `Definition`, `ItemId`, `NetState`... 뿐 — Fragment 자체는 복제되지 않는다. UEquipmentInstance도 클라이언트에서 자체 생성된다.
- Fragment 배열 인덱스는 **최대 16개**(NetState의 `FragmentIndex`가 uint8/비트 제한). 초과 설계 금지.
- 복제 상태가 필요한 Fragment는 `FItemFragmentNetState` 파생이 필요하다 → `a1-replication` 참조.
- 슬롯 태그는 `Equipment.Slot.*`, `QuickBar.Slot.*`. `EEquipmentSlotType` / `EArmorType`(`A1Define.h`)와 대응 관계를 깨지 말 것.
- 장착 초기화는 비동기(에셋 로딩)라 `PendingInitTasks`로 코루틴 관리 중. 동기 가정 코드를 넣지 않는다.
- 캐릭터 간 전송(`Transfer*Server`)은 같은 캐릭터 내 이동(`MoveItemServer`/`EquipFromInventoryServer`/`UnequipToInventoryServer`)과 분리되어 있다. 되돌릴 빈 칸이 없으면 전체 작업을 취소한다(부분 적용 금지) — 새 이동 경로를 추가할 때도 이 원칙을 따른다.
- 아이템/장비 드롭은 `UA1Ability_DropItem`(GameplayEvent 트리거, `UItemInstance*`가 아니라 `ItemId`로 서버에 전달)이 담당한다. 줍기·루팅은 `UA1Ability_Interact_Pickup` / `UA1Ability_Interact_Player`(`Source/A1/AbilitySystem/Interaction`) 쪽 로직이다.
