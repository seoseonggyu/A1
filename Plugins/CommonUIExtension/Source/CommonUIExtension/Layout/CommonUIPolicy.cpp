// Copyright Epic Games, Inc. All Rights Reserved.

#include "Layout/CommonUIPolicy.h"
#include "Layout/CommonPrimaryGameLayout.h"
#include "Game/CommonLocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(CommonUIPolicy)

DEFINE_LOG_CATEGORY(CommonUIPolicyLog);

UCommonUIPolicy::UCommonUIPolicy(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

UCommonPrimaryGameLayout* UCommonUIPolicy::GetRootLayout(const UCommonLocalPlayer* LocalPlayer) const
{
	if (const TObjectPtr<UCommonPrimaryGameLayout>* FoundLayout = PlayerLayoutMap.Find(LocalPlayer))
	{
		return *FoundLayout;
	}

	return nullptr;
}

void UCommonUIPolicy::NotifyPlayerAdded(UCommonLocalPlayer* LocalPlayer)
{
	if (!LocalPlayer)
	{
		return;
	}

	// PlayerController가 설정될 때 레이아웃을 생성하도록 델리게이트에 바인딩합니다
	LocalPlayer->OnPlayerControllerSet.AddWeakLambda(this, [this](UCommonLocalPlayer* LP, const APlayerController* PC)
	{
		// 기존 레이아웃이 있으면 제거합니다
		NotifyPlayerRemoved(LP);

		// 새 레이아웃을 생성합니다
		if (PC)
		{
			CreateLayoutWidget(LP);
		}
	});

	// PlayerController가 이미 있으면 즉시 레이아웃을 생성합니다
	if (LocalPlayer->GetPlayerController(GetWorld()))
	{
		CreateLayoutWidget(LocalPlayer);
	}
}

void UCommonUIPolicy::NotifyPlayerRemoved(UCommonLocalPlayer* LocalPlayer)
{
	if (!LocalPlayer)
	{
		return;
	}

	if (TObjectPtr<UCommonPrimaryGameLayout>* FoundLayout = PlayerLayoutMap.Find(LocalPlayer))
	{
		if (UCommonPrimaryGameLayout* Layout = *FoundLayout)
		{
			RemoveLayoutFromViewport(LocalPlayer, Layout);
		}

		PlayerLayoutMap.Remove(LocalPlayer);
	}
}

void UCommonUIPolicy::CreateLayoutWidget(UCommonLocalPlayer* LocalPlayer)
{
	if (!LayoutWidgetClass)
	{
		UE_LOG(CommonUIPolicyLog, Warning, TEXT("레이아웃 위젯 클래스가 지정되지 않았습니다"));
		return;
	}

	// 이미 레이아웃이 존재하면 생성하지 않습니다
	if (PlayerLayoutMap.Contains(LocalPlayer))
	{
		return;
	}

	APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld());
	if (!PC)
	{
		// PlayerController가 없으면 나중에 OnPlayerControllerSet 델리게이트가 호출될 때 생성됩니다
		return;
	}

	if (UCommonPrimaryGameLayout* Layout = CreateWidget<UCommonPrimaryGameLayout>(PC, LayoutWidgetClass))
	{
		PlayerLayoutMap.Add(LocalPlayer, Layout);
		AddLayoutToViewport(LocalPlayer, Layout);
	}
}

void UCommonUIPolicy::AddLayoutToViewport(UCommonLocalPlayer* LocalPlayer, UCommonPrimaryGameLayout* Layout) const
{
	UE_LOG(CommonUIPolicyLog, Log, TEXT("LocalPlayer '%s'의 레이아웃 '%s'을 뷰포트에 추가합니다"), *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));

	Layout->SetPlayerContext(FLocalPlayerContext(LocalPlayer));
	Layout->AddToPlayerScreen(1000);
}

void UCommonUIPolicy::RemoveLayoutFromViewport(const UCommonLocalPlayer* LocalPlayer, UCommonPrimaryGameLayout* Layout) const
{
	if (Layout && Layout->IsInViewport())
	{
		Layout->RemoveFromParent();
	}
}
