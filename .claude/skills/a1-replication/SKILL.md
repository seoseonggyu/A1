---
name: a1-replication
description: A1 프로젝트에서 네트워크 복제를 다룰 때 반드시 사용한다. Iris, FastArraySerializer, FIrisFastArraySerializer, NetSerializer, ItemFragmentNetState, 다형성 구조체 복제, RPC, Replicated 프로퍼티, OnRep, 서버/클라 권한 분리 관련 작업이면 이 스킬을 확인할 것. 새 복제 프로퍼티나 리스트를 추가할 때도 반드시 볼 것.
---

# 네트워크 복제 (A1 / Iris)

프로젝트는 **Iris** 복제를 사용한다 (`SetupIrisSupport(Target)`). 리스트류는 전부 `FIrisFastArraySerializer` 기반.

## 권한 규칙

| 접미사 | 실행 위치 | 표기 |
|---|---|---|
| `...Auth` | 서버 전용 | `UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)` |
| `...Server` | 클라 → 서버 RPC | `UFUNCTION(Server, Reliable)` |
| `...Local` | 호출한 쪽만 | 비복제 |

- 상태 변경은 **항상 서버에서** 시작한다. 클라이언트는 요청(RPC)만 보낸다.
- `Auth` 함수 진입부에서 `if (!GetOwner()->HasAuthority()) return;` 형태의 방어를 유지한다.

## 프로퍼티 추가 절차

1. `UPROPERTY(Replicated)` 또는 `ReplicatedUsing = OnRep_<Name>`
2. `GetLifetimeReplicatedProps`에 `DOREPLIFETIME(...)` / `DOREPLIFETIME_CONDITION(...)` 추가 — **누락 시 조용히 복제 안 됨**
3. Owner에게만 필요한 데이터는 `COND_OwnerOnly` (예: 인벤토리)
4. `OnRep_`은 클라이언트에서만 불린다는 전제로 작성

## FastArray 리스트 추가

```cpp
USTRUCT()
struct F<Name>Entry : public FFastArraySerializerItem
{
    GENERATED_BODY()
    void PostReplicatedAdd(const F<Name>List& S);
    void PostReplicatedChange(const F<Name>List& S);
    void PreReplicatedRemove(const F<Name>List& S);
};

USTRUCT()
struct F<Name>List : public FIrisFastArraySerializer
{
    GENERATED_BODY()

    bool NetDeltaSerialize(FNetDeltaSerializeInfo& P)
    {
        return FFastArraySerializer::FastArrayDeltaSerialize<F<Name>Entry, F<Name>List>(Entries, P, *this);
    }
    void MarkEntryDirty(F<Name>Entry& E) { MarkItemDirty(E); }

    UPROPERTY() TArray<F<Name>Entry> Entries;
    UPROPERTY(NotReplicated) TObjectPtr<U<Owner>Component> Owner = nullptr;
};

template<> struct TStructOpsTypeTraits<F<Name>List> : public TStructOpsTypeTraitsBase2<F<Name>List>
{
    enum { WithNetDeltaSerializer = true };
};
```

- Entry 수정 후 `MarkEntryDirty(Entry)` 호출 필수.
- **FastArray를 중첩하지 말 것.** (그래서 아이템 NetState가 `FInventoryEntry` 안이 아니라 `FItemNetStateList`로 컴포넌트 최상위에 분리되어 있다.)
- UObject 인스턴스는 복제하지 않고 `PostReplicatedAdd`에서 클라이언트가 로컬 생성하는 패턴을 따른다 (`UItemInstance`, `UEquipmentInstance`).

## 다형성 구조체 복제 (NetState)

```cpp
USTRUCT()
struct FNetState_<이름> : public FItemFragmentNetState
{
    GENERATED_BODY()
    virtual const UScriptStruct* GetScriptStruct() const override { return StaticStruct(); }  // 필수
};
```

- 컨테이너는 `FItemFragmentNetStateHandle`(TSharedPtr 기반, `TPolymorphicStructNetSerializerImpl`).
- 서버 수정: `Instance->ModifyNetStateAuth<TFragment>()` / `ModifyTagStatAuth(Tag)` → 수정자 소멸 시 dirty 처리.
- Fragment 인덱스는 **최대 16개** 제한.
- 새 NetSerializer가 필요하면 `Inventory/Fragment/NetState/Serialization/`의 기존 구현을 참고해 모듈 등록까지 함께 처리한다.

## 체크리스트

- [ ] `GetLifetimeReplicatedProps` 갱신
- [ ] `MarkEntryDirty` 호출
- [ ] 서버/클라 분기 및 `HasAuthority` 방어
- [ ] 로컬 예측이 있다면 서버 확정값으로 되돌아오는 경로 확인
