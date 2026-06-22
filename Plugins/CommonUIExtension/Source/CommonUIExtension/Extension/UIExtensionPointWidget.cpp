// Copyright Epic Games, Inc. All Rights Reserved.

#include "Extension/UIExtensionPointWidget.h"
#include "Extension/UIExtensionSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(UIExtensionPointWidget)

DEFINE_LOG_CATEGORY(UIExtensionPointWidgetLog);

UUIExtensionPointWidget::UUIExtensionPointWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UUIExtensionPointWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	// 디자인 타임에는 등록하지 않았으므로 해제도 불필요합니다
	if (IsDesignTime())
	{
		return;
	}

	// 위젯 리소스 해제 시 등록 해제합니다
	if (ExtensionPointHandle.IsValid())
	{
		if (UUIExtensionSubsystem* ExtensionSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UUIExtensionSubsystem>() : nullptr)
		{
			ExtensionSubsystem->UnregisterExtensionPoint(ExtensionPointHandle);
		}
		
		ExtensionPointHandle.Reset();
	}

	ExtensionWidgets.Empty();
}

void UUIExtensionPointWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	// 디자인 타임에는 등록하지 않습니다 (미리보기만 표시)
	if (IsDesignTime())
	{
		return;
	}

	// 이미 등록되어 있으면 스킵합니다
	if (ExtensionPointHandle.IsValid())
	{
		return;
	}

	if (!ExtensionPointTag.IsValid())
	{
		return;
	}

	// 확장 포인트를 등록합니다
	if (UUIExtensionSubsystem* ExtensionSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UUIExtensionSubsystem>() : nullptr)
	{
		TArray<UClass*> AllowedClasses;
		for (UClass* DataClass : DataClasses)
		{
			if (DataClass)
			{
				AllowedClasses.Add(DataClass);
			}
		}

		ExtensionPointHandle = ExtensionSubsystem->RegisterExtensionPoint(
			ExtensionPointTag,
			ExtensionPointTagMatch,
			AllowedClasses,
			FExtendExtensionPointDelegate::CreateUObject(this, &ThisClass::OnExtensionAction)
		);
	}
}

void UUIExtensionPointWidget::BeginDestroy()
{
	// 확장 포인트 등록을 해제합니다
	if (ExtensionPointHandle.IsValid())
	{
		if (UUIExtensionSubsystem* ExtensionSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UUIExtensionSubsystem>() : nullptr)
		{
			ExtensionSubsystem->UnregisterExtensionPoint(ExtensionPointHandle);
		}
		ExtensionPointHandle.Reset();
	}

	ExtensionWidgets.Empty();

	Super::BeginDestroy();
}

#if WITH_EDITOR
const FText UUIExtensionPointWidget::GetPaletteCategory()
{
	return NSLOCTEXT("UIExtension", "PaletteCategory", "UI Extension");
}
#endif

TSharedRef<SWidget> UUIExtensionPointWidget::RebuildWidget()
{
	// 에디터 미리보기용 (디자인 타임에만 표시)
	if (IsDesignTime())
	{
		const bool bHasValidTag = ExtensionPointTag.IsValid();

		// 태그 설정됨: 초록색 / 태그 미설정: 빨간색
		const FLinearColor BorderColor = bHasValidTag
			? FLinearColor(0.1f, 0.8f, 0.1f, 1.0f)
			: FLinearColor(0.8f, 0.1f, 0.1f, 1.0f);

		const FString DisplayText = bHasValidTag
			? ExtensionPointTag.ToString()
			: TEXT("[Extension Point - Tag 미설정]");

		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("Border"))
			.BorderBackgroundColor(BorderColor)
			.Padding(4.0f)
			[
				SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(DisplayText))
					.ColorAndOpacity(BorderColor)
				]
			];
	}

	return Super::RebuildWidget();
}

void UUIExtensionPointWidget::OnExtensionAction(EUIExtensionAction Action, const FUIExtensionRequest& Request)
{
	switch (Action)
	{
	case EUIExtensionAction::Added:
		AddExtensionWidget(Request);
		break;

	case EUIExtensionAction::Removed:
		RemoveExtensionWidget(Request);
		break;
	}
}

void UUIExtensionPointWidget::AddExtensionWidget(const FUIExtensionRequest& Request)
{
	// 이미 위젯이 존재하는지 확인합니다
	if (ExtensionWidgets.Contains(Request.ExtensionHandle))
	{
		UE_LOG(UIExtensionPointWidgetLog, Warning, TEXT("이미 존재하는 확장 위젯입니다"));
		return;
	}

	// 위젯 클래스를 결정합니다
	TSubclassOf<UUserWidget> WidgetClass = GetWidgetClassForRequest(Request);
	if (!WidgetClass)
	{
		UE_LOG(UIExtensionPointWidgetLog, Warning, TEXT("위젯 클래스를 결정할 수 없습니다"));
		return;
	}

	// 위젯을 생성합니다
	UUserWidget* NewWidget = CreateEntryInternal(WidgetClass);
	if (!NewWidget)
	{
		UE_LOG(UIExtensionPointWidgetLog, Warning, TEXT("위젯 생성 실패: %s"), *WidgetClass->GetName());
		return;
	}

	// 위젯을 초기화합니다
	if (ConfigureWidgetForData.IsBound())
	{
		ConfigureWidgetForData.Execute(NewWidget, Request);
	}

	ExtensionWidgets.Add(Request.ExtensionHandle, NewWidget);
}

void UUIExtensionPointWidget::RemoveExtensionWidget(const FUIExtensionRequest& Request)
{
	TObjectPtr<UUserWidget>* FoundWidget = ExtensionWidgets.Find(Request.ExtensionHandle);
	if (!FoundWidget || !*FoundWidget)
	{
		return;
	}

	UUserWidget* Widget = *FoundWidget;

	// DynamicEntryBox의 엔트리 제거 API를 사용합니다
	RemoveEntryInternal(Widget);

	ExtensionWidgets.Remove(Request.ExtensionHandle);
}

TSubclassOf<UUserWidget> UUIExtensionPointWidget::GetWidgetClassForRequest(const FUIExtensionRequest& Request) const
{
	// 델리게이트가 바인딩되어 있으면 사용합니다
	if (GetWidgetClassForData.IsBound())
	{
		return GetWidgetClassForData.Execute(Request);
	}

	// Data가 UClass(위젯 클래스)인 경우 직접 사용합니다
	if (UClass* DataClass = Cast<UClass>(Request.Data))
	{
		if (DataClass->IsChildOf(UUserWidget::StaticClass()))
		{
			return DataClass;
		}
	}

	return nullptr;
}
