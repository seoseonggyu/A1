// Copyright Epic Games, Inc. All Rights Reserved.

#include "Extension/UIExtensionSubsystem.h"
#include "Blueprint/UserWidget.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(UIExtensionSubsystem)

DEFINE_LOG_CATEGORY(UIExtensionSubsystemLog);

UUIExtensionSubsystem::UUIExtensionSubsystem()
{
}

void UUIExtensionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UUIExtensionSubsystem::Deinitialize()
{
	ExtensionPoints.Empty();
	Extensions.Empty();

	Super::Deinitialize();
}

FUIExtensionPointHandle UUIExtensionSubsystem::RegisterExtensionPoint(FGameplayTag ExtensionPointTag, EUIExtensionPointMatch MatchType, const TArray<UClass*>& AllowedDataClasses, const FExtendExtensionPointDelegate& Callback)
{
	if (!ExtensionPointTag.IsValid())
	{
		UE_LOG(UIExtensionSubsystemLog, Warning, TEXT("확장 포인트 등록 실패: 유효하지 않은 태그"));
		return FUIExtensionPointHandle();
	}

	int32 PointId = GetNextExtensionPointId();

	FUIExtensionPoint& NewPoint = ExtensionPoints.Add(PointId);
	NewPoint.ExtensionPointTag = ExtensionPointTag;
	NewPoint.MatchType = MatchType;
	NewPoint.AllowedDataClasses = AllowedDataClasses;
	NewPoint.Callback = Callback;

	// 기존 확장들과 매칭을 시도합니다
	MatchPointToExtension(PointId, NewPoint);
	
	return FUIExtensionPointHandle(PointId);
}

void UUIExtensionSubsystem::UnregisterExtensionPoint(FUIExtensionPointHandle Handle)
{
	if (!Handle.IsValid())
	{
		return;
	}

	FUIExtensionPoint* Point = ExtensionPoints.Find(Handle.ExtensionPointId);
	if (!Point)
	{
		return;
	}

	// 연결된 Extension들의 ConnectedPointIds에서 이 Point를 제거합니다
	for (int32 ExtensionId : Point->ConnectedExtensionIds)
	{
		if (FUIExtension* Extension = Extensions.Find(ExtensionId))
		{
			Extension->ConnectedPointIds.Remove(Handle.ExtensionPointId);
		}
	}
	
	ExtensionPoints.Remove(Handle.ExtensionPointId);
}

FUIExtensionHandle UUIExtensionSubsystem::RegisterExtensionAsWidget(FGameplayTag ExtensionPointTag, TSubclassOf<UUserWidget> WidgetClass, int32 Priority)
{
	if (!ExtensionPointTag.IsValid() || !WidgetClass)
	{
		UE_LOG(UIExtensionSubsystemLog, Warning, TEXT("위젯 확장 등록 실패: 유효하지 않은 태그 또는 클래스"));
		return FUIExtensionHandle();
	}

	int32 ExtensionId = GetNextExtensionId();

	FUIExtension& NewExtension = Extensions.Add(ExtensionId);
	NewExtension.ExtensionPointTag = ExtensionPointTag;
	NewExtension.Priority = Priority;
	NewExtension.Data = WidgetClass;

	// 기존 확장 포인트들과 매칭을 시도합니다
	MatchExtensionToPoint(ExtensionId, NewExtension);
	
	return FUIExtensionHandle(ExtensionId);
}

FUIExtensionHandle UUIExtensionSubsystem::RegisterExtensionAsData(FGameplayTag ExtensionPointTag, UObject* ContextObject, UObject* Data, int32 Priority)
{
	if (!ExtensionPointTag.IsValid() || !Data)
	{
		UE_LOG(UIExtensionSubsystemLog, Warning, TEXT("데이터 확장 등록 실패: 유효하지 않은 태그 또는 데이터"));
		return FUIExtensionHandle();
	}

	int32 ExtensionId = GetNextExtensionId();

	FUIExtension& NewExtension = Extensions.Add(ExtensionId);
	NewExtension.ExtensionPointTag = ExtensionPointTag;
	NewExtension.Priority = Priority;
	NewExtension.Data = Data;
	NewExtension.ContextObject = ContextObject;

	// 기존 확장 포인트들과 매칭을 시도합니다
	MatchExtensionToPoint(ExtensionId, NewExtension);

	UE_LOG(UIExtensionSubsystemLog, Log, TEXT("데이터 확장 등록: %s → %s (ID: %d)"), *Data->GetName(), *ExtensionPointTag.ToString(), ExtensionId);

	return FUIExtensionHandle(ExtensionId);
}

void UUIExtensionSubsystem::UnregisterExtension(FUIExtensionHandle Handle)
{
	if (!Handle.IsValid())
	{
		return;
	}

	FUIExtension* Extension = Extensions.Find(Handle.ExtensionId);
	if (!Extension)
	{
		return;
	}

	// 연결된 확장 포인트들에게 제거 알림을 보냅니다
	FUIExtensionRequest Request = CreateExtensionRequest(Handle.ExtensionId, *Extension);

	for (int32 PointId : Extension->ConnectedPointIds)
	{
		if (FUIExtensionPoint* Point = ExtensionPoints.Find(PointId))
		{
			Point->ConnectedExtensionIds.Remove(Handle.ExtensionId);
			if (Point->Callback.IsBound())
			{
				Point->Callback.Execute(EUIExtensionAction::Removed, Request);
			}
		}
	}
	
	Extensions.Remove(Handle.ExtensionId);
}

void UUIExtensionSubsystem::MatchExtensionToPoint(int32 ExtensionId, FUIExtension& Extension)
{
	for (auto& [PointId, Point] : ExtensionPoints)
	{
		if (!DoesTagMatch(Extension.ExtensionPointTag, Point.ExtensionPointTag, Point.MatchType))
		{
			continue;
		}

		if (!IsDataClassAllowed(Extension.Data.IsValid() ? Extension.Data->GetClass() : nullptr, Point.AllowedDataClasses))
		{
			continue;
		}

		// 매칭 성공 - 연결합니다
		Extension.ConnectedPointIds.AddUnique(PointId);
		Point.ConnectedExtensionIds.AddUnique(ExtensionId);

		// 콜백을 호출합니다
		if (Point.Callback.IsBound())
		{
			FUIExtensionRequest Request = CreateExtensionRequest(ExtensionId, Extension);
			Point.Callback.Execute(EUIExtensionAction::Added, Request);
		}
	}
}

void UUIExtensionSubsystem::MatchPointToExtension(int32 PointId, FUIExtensionPoint& Point)
{
	for (auto& [ExtensionId, Extension] : Extensions)
	{
		if (!DoesTagMatch(Extension.ExtensionPointTag, Point.ExtensionPointTag, Point.MatchType))
		{
			continue;
		}

		if (!IsDataClassAllowed(Extension.Data.IsValid() ? Extension.Data->GetClass() : nullptr, Point.AllowedDataClasses))
		{
			continue;
		}

		// 매칭 성공 - 연결합니다
		Extension.ConnectedPointIds.AddUnique(PointId);
		Point.ConnectedExtensionIds.AddUnique(ExtensionId);

		// 콜백을 호출합니다
		if (Point.Callback.IsBound())
		{
			FUIExtensionRequest Request = CreateExtensionRequest(ExtensionId, Extension);
			Point.Callback.Execute(EUIExtensionAction::Added, Request);
		}
	}
}

bool UUIExtensionSubsystem::DoesTagMatch(FGameplayTag ExtensionTag, FGameplayTag PointTag, EUIExtensionPointMatch MatchType) const
{
	switch (MatchType)
	{
	case EUIExtensionPointMatch::ExactMatch:
		return ExtensionTag.MatchesTagExact(PointTag);

	case EUIExtensionPointMatch::PartialMatch:
		return ExtensionTag.MatchesTag(PointTag);

	default:
		return false;
	}
}

bool UUIExtensionSubsystem::IsDataClassAllowed(const UClass* DataClass, const TArray<UClass*>& AllowedClasses) const
{
	// 허용 클래스가 비어있으면 모두 허용합니다
	if (AllowedClasses.IsEmpty())
	{
		return true;
	}

	if (!DataClass)
	{
		return false;
	}

	for (UClass* AllowedClass : AllowedClasses)
	{
		if (DataClass->IsChildOf(AllowedClass))
		{
			return true;
		}
	}

	return false;
}

FUIExtensionRequest UUIExtensionSubsystem::CreateExtensionRequest(int32 ExtensionId, const FUIExtension& Extension) const
{
	FUIExtensionRequest Request;
	Request.ExtensionHandle = FUIExtensionHandle(ExtensionId);
	Request.ExtensionPointTag = Extension.ExtensionPointTag;
	Request.Priority = Extension.Priority;
	Request.Data = Extension.Data.Get();
	Request.ContextObject = Extension.ContextObject.Get();
	return Request;
}
