// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Skill/A1SkillCooldownWidget.h"

#include "AbilitySystemComponent.h"
#include "Game/CommonCharacter.h"
#include "Equipment/EquipmentComponent.h"
#include "Equipment/EquipmentInstance.h"
#include "Equipment/EquipmentDefinition.h"
#include "Equipment/Fragment/EquipmentFragment_Ability.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1SkillCooldownWidget)

DEFINE_LOG_CATEGORY(A1SkillCooldownWidgetLog);

void UA1SkillCooldownWidget::NativeConstruct()
{
	Super::NativeConstruct();

	TryStartTracking();
}

void UA1SkillCooldownWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RetryTimerHandle);
	}

	TearDown();

	Super::NativeDestruct();
}

void UA1SkillCooldownWidget::TryStartTracking()
{
	if (SetupTracking())
	{
		return;
	}

	// 이 위젯은 Inventory/Equipment 창과 달리 HUD에 항상 떠 있어서, Pawn 빙의나 ASC/Equipment
	// 컴포넌트 리플리케이션(특히 클라이언트)이 끝나기 전에 NativeConstruct가 먼저 실행될 수 있다.
	// 준비될 때까지 짧은 간격으로 재시도한다 (성공하면 이 타이머는 더 이상 갱신되지 않고 끝난다).
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RetryTimerHandle, this, &ThisClass::TryStartTracking, 0.2f, false);
	}
}

bool UA1SkillCooldownWidget::SetupTracking()
{
	// 재시도 시 이전 부분 구독이 남아있지 않도록 먼저 정리합니다 (매번 처음부터 다시 구독).
	TearDown();

	ACommonCharacter* Character = Cast<ACommonCharacter>(GetOwningPlayerPawn());
	if (!Character)
	{
		return false;
	}

	UAbilitySystemComponent* InASC = Character->GetAbilitySystemComponent();
	UEquipmentComponent* InEquipmentComponent = UEquipmentComponent::FindEquipmentComponent(Character);
	if (!InASC || !InEquipmentComponent)
	{
		return false;
	}

	ASC = InASC;
	EquipmentComponent = InEquipmentComponent;

	if (CooldownTag.IsValid())
	{
		InASC->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::HandleCooldownTagChanged);

		// 초기 상태 반영 (이미 쿨타임 중일 수 있음)
		HandleCooldownTagChanged(CooldownTag, InASC->GetTagCount(CooldownTag));
	}

	MainEquippedChangedHandle = InEquipmentComponent->OnMainEquippedItemChanged.AddUObject(this, &ThisClass::HandleMainEquippedItemChanged);

	// 초기 상태 반영 (이미 무언가 손에 들려 있을 수 있음)
	HandleMainEquippedItemChanged(InEquipmentComponent->GetActiveMainEquippedItem());

	return true;
}

void UA1SkillCooldownWidget::TearDown()
{
	if (UAbilitySystemComponent* ASCPtr = ASC.Get())
	{
		if (CooldownTag.IsValid())
		{
			ASCPtr->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
		}
	}

	if (UEquipmentComponent* EquipmentComponentPtr = EquipmentComponent.Get())
	{
		EquipmentComponentPtr->OnMainEquippedItemChanged.Remove(MainEquippedChangedHandle);
	}
	MainEquippedChangedHandle.Reset();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CooldownTimerHandle);
	}

	ASC.Reset();
	EquipmentComponent.Reset();
}

void UA1SkillCooldownWidget::HandleMainEquippedItemChanged(UEquipmentInstance* NewMainItem)
{
	bHasSkill = false;
	CurrentIcon = nullptr;

	const UEquipmentDefinition* Definition = NewMainItem ? NewMainItem->GetDefinition() : nullptr;
	if (const FEquipmentFragment_Ability* Fragment = Definition ? Definition->FindFragment<FEquipmentFragment_Ability>() : nullptr)
	{
		for (const FCommonAbilityEntry& Entry : Fragment->Abilities)
		{
			if (Entry.InputTag == SkillInputTag)
			{
				bHasSkill = true;
				// 검/활 등 장착 아이템마다 EquipmentDefinition에 지정된 아이콘을 그대로 사용합니다.
				CurrentIcon = Entry.Icon;
				break;
			}
		}
	}

	RefreshVisual();
}

void UA1SkillCooldownWidget::HandleCooldownTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	UWorld* World = GetWorld();

	if (NewCount > 0)
	{
		bOnCooldown = true;
		CooldownRemaining = FMath::CeilToInt(GetRemainingCooldownSeconds());

		if (World)
		{
			World->GetTimerManager().SetTimer(CooldownTimerHandle, this, &ThisClass::TickCooldown, 1.f, true);
		}
	}
	else
	{
		if (World)
		{
			World->GetTimerManager().ClearTimer(CooldownTimerHandle);
		}

		bOnCooldown = false;
		CooldownRemaining = 0;
	}

	RefreshVisual();
}

void UA1SkillCooldownWidget::TickCooldown()
{
	const float Remaining = GetRemainingCooldownSeconds();

	if (Remaining <= 0.f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(CooldownTimerHandle);
		}

		bOnCooldown = false;
		CooldownRemaining = 0;
		RefreshVisual();
		return;
	}

	CooldownRemaining = FMath::CeilToInt(Remaining);
	RefreshVisual();
}

float UA1SkillCooldownWidget::GetRemainingCooldownSeconds() const
{
	UAbilitySystemComponent* ASCPtr = ASC.Get();
	if (!ASCPtr || !CooldownTag.IsValid())
	{
		return 0.f;
	}

	// CooldownTag를 부여(GrantedTags)하는 활성 GameplayEffect를 찾아 남은 시간을 조회합니다.
	const FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(CooldownTag));
	const TArray<float> TimesRemaining = ASCPtr->GetActiveEffectsTimeRemaining(Query);

	float MaxRemaining = 0.f;
	for (const float Remaining : TimesRemaining)
	{
		MaxRemaining = FMath::Max(MaxRemaining, Remaining);
	}
	return MaxRemaining;
}

void UA1SkillCooldownWidget::RefreshVisual()
{
	if (Image_Icon)
	{
		// 스킬이 없으면 완전히 숨기고, 있으면 쿨타임 여부에 따라 흐리게/선명하게 표시합니다.
		Image_Icon->SetVisibility(bHasSkill ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		Image_Icon->SetRenderOpacity(bOnCooldown ? CooldownOpacity : 1.f);

		if (bHasSkill && CurrentIcon)
		{
			Image_Icon->SetBrushFromTexture(CurrentIcon);
		}
	}

	if (Text_Cooldown)
	{
		const bool bShowCooldownText = bHasSkill && bOnCooldown;
		Text_Cooldown->SetVisibility(bShowCooldownText ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		Text_Cooldown->SetText(bShowCooldownText ? FText::AsNumber(CooldownRemaining) : FText::GetEmpty());
	}
}
