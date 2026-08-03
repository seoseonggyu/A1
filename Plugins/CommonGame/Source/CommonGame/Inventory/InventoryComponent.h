// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ControllerComponent.h"
#include "Inventory/ItemInstance.h"
#include "Inventory/Fragment/NetState/ItemFragmentNetStateList.h"
#include "Coro.h"
#include "InventoryComponent.generated.h"

struct FInventoryList;
class UItemDefinition;
class UExperienceDefinition;
class UInventoryComponent;

DECLARE_LOG_CATEGORY_EXTERN(InventoryComponentLog, Log, All);

/** ������ ���� �� �ʱ�ȭ �Ϸ� ��������Ʈ */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryItemReady, int32 /*ItemId*/, UItemInstance* /*Instance*/);

//-----------------------------------------------------------------------------
// FInitialItemEntry
//-----------------------------------------------------------------------------

/**
 * �ʱ� ������ ���� �׸�
 *
 * ���� ���� �� �ڵ����� �߰��� �������� �����մϴ�.
 */
USTRUCT(BlueprintType)
struct COMMONGAME_API FInitialItemEntry
{
	GENERATED_BODY()

public:
	bool IsValid() const { return ItemDefinition != nullptr && Count > 0; }

public:
	/** �߰��� ������ ���� */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TObjectPtr<const UItemDefinition> ItemDefinition;

	/** �߰��� ���� */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "1"))
	int32 Count = 1;

	/** QuickBar���� �߰����� ���� */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	bool bAddToQuickBar = false;
};

//-----------------------------------------------------------------------------
// FInventoryEntry
//-----------------------------------------------------------------------------

/**
 * �κ��丮 �׸�
 *
 * ������ �����Ϳ� ��Ÿ�� �ν��Ͻ��� ����ϴ�.
 * ���ø����̼� ������(Definition, ItemId)�� Entry�� ���� ���Եǰ�,
 * UItemInstance�� Ŭ���̾�Ʈ���� ���÷� �����˴ϴ�.
 * NetState�� ��ø FastArraySerializer�� ���ϱ� ���� UInventoryComponent::ItemNetStates���� �����մϴ�.
 */
USTRUCT(BlueprintType)
struct COMMONGAME_API FInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

public:
	void PostReplicatedAdd(const FInventoryList& InArraySerializer);
	void PostReplicatedChange(const FInventoryList& InArraySerializer);
	void PreReplicatedRemove(const FInventoryList& InArraySerializer);

	void NotifyCreated(UInventoryComponent* Owner) const;
	void NotifyChanged(UInventoryComponent* Owner) const;
	void NotifyRemoved(UInventoryComponent* Owner) const;

public:
	//-----------------------------------------------------------------------------
	// ���ø����̼� ������
	//-----------------------------------------------------------------------------

	/** ������ ���� */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<const UItemDefinition> Definition = nullptr;

	/** ���� ������ ID */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 ItemId = INDEX_NONE;

	/** ���� ���� */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 StackCount = 1;

	//-----------------------------------------------------------------------------
	// ��Ÿ�� ������ (���ø����̼� �� ��)
	//-----------------------------------------------------------------------------

	/** ������ �ν��Ͻ� (����: ���� ����, Ŭ���̾�Ʈ: PostReplicatedAdd���� ���� ����) */
	UPROPERTY(NotReplicated, Transient, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UItemInstance> Instance = nullptr;
};

//-----------------------------------------------------------------------------
// FInventoryList
//-----------------------------------------------------------------------------

/**
 * �κ��丮 �׸� �迭�� ��� FastArraySerializer �����̳�
 *
 * ��Ʈ��ũ ��Ÿ ����ȭ�� �����մϴ�.
 */
USTRUCT(BlueprintType)
struct COMMONGAME_API FInventoryList : public FIrisFastArraySerializer
{
	GENERATED_BODY()

public:
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryEntry, FInventoryList>(Entries, DeltaParms, *this);
	}

	/** Entry�� Dirty�� ��ŷ�մϴ� */
	void MarkEntryDirty(FInventoryEntry& Entry)
	{
		MarkItemDirty(Entry);
	}

public:
	/** �κ��丮 �׸� �迭 */
	UPROPERTY()
	TArray<FInventoryEntry> Entries;

	/** ���� ������Ʈ (�������� ����) */
	UPROPERTY(NotReplicated)
	TObjectPtr<UInventoryComponent> Owner = nullptr;
};

template<>
struct TStructOpsTypeTraits<FInventoryList> : public TStructOpsTypeTraitsBase2<FInventoryList>
{
	enum { WithNetDeltaSerializer = true };
};

//-----------------------------------------------------------------------------
// UInventoryComponent
//-----------------------------------------------------------------------------

/**
 * �÷��̾� �κ��丮 ������Ʈ
 *
 * PlayerController�� �پ �������� �����մϴ�.
 * ��Ʈ��ũ ������ �����ϸ�, Owner���Ը� �����˴ϴ�.
 * Experience �ε� �Ϸ� �� �ʱ� �������� �ڵ����� �߰��մϴ�.
 */
UCLASS(ClassGroup = (Inventory), meta = (BlueprintSpawnableComponent))
class COMMONGAME_API UInventoryComponent : public UControllerComponent
{
	GENERATED_BODY()

	friend struct FInventoryEntry;
	friend struct FItemNetStateEntry;
	friend class UItemInstance;

public:
	UInventoryComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static UInventoryComponent* FindInventoryComponent(const APawn* Pawn);
	static UInventoryComponent* FindInventoryComponent(const AController* Controller);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	//-----------------------------------------------------------------------------
	// ������ ���� (���� ����)
	//-----------------------------------------------------------------------------

	/** �������� �߰��մϴ� (���� ����, �ڷ�ƾ) */
	TCoroTask<UItemInstance*> AddItemAuthCoroutine(const UItemDefinition* Definition, int32 Count = 1);

	/** �������� �����մϴ� (���� ����) */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	bool RemoveItemAuth(UItemInstance* Instance);

	/** ���� ������ �����մϴ� (���� ����) */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	bool ModifyStackCountAuth(UItemInstance* Instance, int32 NewCount);

	/** TagStat ���� �����մϴ� (Ŭ���̾�Ʈ �� ���� RPC) */
	UFUNCTION(Server, Reliable)
	void ModifyTagStatServer(int32 ItemId, FGameplayTag StatTag, float NewValue);

	//-----------------------------------------------------------------------------
	// �ʱ� ������ (���� ����)
	//-----------------------------------------------------------------------------

	/** �ʱ� �������� �����մϴ� (���� ����, Pawn Possess �� ȣ��) */
	UFUNCTION()
	void GiveInitialItemsAuth(APawn* OldPawn, APawn* NewPawn);

	//-----------------------------------------------------------------------------
	// ������ ��ȸ
	//-----------------------------------------------------------------------------

	/** Ư�� ������ �������� ã���ϴ� */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UItemInstance* FindItemAtSlot(int32 SlotIndex) const;

	/** ID�� �������� ã���ϴ� */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UItemInstance* FindItemById(int32 ItemId) const;

	/** ��� �������� ��ȯ�մϴ� */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void GetAllItems(TArray<UItemInstance*>& OutItems) const;

	/** ������ ������ ��ȯ�մϴ� */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemCount() const { return InventoryList.Entries.Num(); }

	/** �ִ� ���� ���� ��ȯ�մϴ� */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetMaxSlots() const { return MaxSlots; }

	/** ������ ���� á���� Ȯ���մϴ� */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool IsSlotFull() const { return GetItemCount() >= MaxSlots; }

	/** �������� Entry�� ã���ϴ� */
	FInventoryEntry* FindEntry(const UItemInstance* Instance);
	const FInventoryEntry* FindEntry(const UItemInstance* Instance) const;

	/** ������ ���� �� �ʱ�ȭ �Ϸ� ��������Ʈ (����/Ŭ���̾�Ʈ ���) */
	FOnInventoryItemReady OnItemReady;

	/** Ư�� Fragment�� ���� �������� ã���ϴ� */
	template<typename T>
	UItemInstance* FindItemWithFragment() const
	{
		for (const FInventoryEntry& Entry : InventoryList.Entries)
		{
			if (Entry.Instance && Entry.Instance->FindFragment<T>())
			{
				return Entry.Instance;
			}
		}
		return nullptr;
	}

	/** Ư�� Fragment�� ���� ��� �������� ã���ϴ� */
	template<typename T>
	TArray<UItemInstance*> FindAllItemsWithFragment() const
	{
		TArray<UItemInstance*> Result;
		for (const FInventoryEntry& Entry : InventoryList.Entries)
		{
			if (Entry.Instance && Entry.Instance->FindFragment<T>())
			{
				Result.Add(Entry.Instance);
			}
		}
		return Result;
	}

	/** ������ Entry�� ���� ���� ItemInstance�� �����ϰ� �ʱ�ȭ�մϴ� (Ŭ���̾�Ʈ ����) */
	TCoroTask<void> InitializeReplicatedItemCoroutine(FInventoryEntry* Entry);

private:
	/**
	 * ���� ������ �߰� ���� (���� �ε� �Ϸ� ���¿��� ȣ��)
	 * @param ReservedIndex �̸� ����� ���� �ε���
	 */
	UItemInstance* AddItemAuthInternal(const UItemDefinition* Definition, int32 Count, int32 ReservedIndex);

protected:
	/** ���ο� ������ ID�� �����մϴ� (���� ����, ���� ��ü���� ����) */
	static int32 GenerateItemId();

protected:
	/** �κ��丮 ��� */
	UPROPERTY(Replicated)
	FInventoryList InventoryList;

	/** ������ NetState ��� (�ֻ��� FastArraySerializer�� ��Ÿ ����) */
	UPROPERTY(Replicated)
	FItemNetStateList ItemNetStates;

	/** �ִ� ���� �� */
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 MaxSlots = 36;

	/** �ʱ� ������ ��� */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Initial Items")
	TArray<FInitialItemEntry> InitialItems;

	/** ������ ID -> Instance �� */
	TMap<int32, TObjectPtr<UItemInstance>> ItemMap;

private:
	/** �ʱ� ������ ���� �Ϸ� ���� */
	bool bInitialItemsGiven = false;
};
