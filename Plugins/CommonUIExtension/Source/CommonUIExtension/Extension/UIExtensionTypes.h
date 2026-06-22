// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "UIExtensionTypes.generated.h"

//-----------------------------------------------------------------------------
// EUIExtensionPointMatch
//-----------------------------------------------------------------------------

/** 확장 포인트 매칭 규칙 */
UENUM(BlueprintType)
enum class EUIExtensionPointMatch : uint8
{
	/** 정확히 일치하는 태그만 매칭 (A.B는 A.B만 매칭) */
	ExactMatch,

	/** 부분 매칭 허용 (A.B는 A.B, A.B.C 모두 매칭) */
	PartialMatch
};

//-----------------------------------------------------------------------------
// FUIExtensionPointHandle
//-----------------------------------------------------------------------------

/**
 * 확장 포인트 핸들 (수신 측)
 *
 * UI 슬롯을 등록할 때 반환되며, 등록 해제에 사용됩니다.
 * 예: HUD에서 "여기에 위젯을 받겠다"고 등록한 슬롯의 핸들
 */
USTRUCT(BlueprintType)
struct COMMONUIEXTENSION_API FUIExtensionPointHandle
{
	GENERATED_BODY()

	friend class UUIExtensionSubsystem;

public:
	FUIExtensionPointHandle() = default;

	/** 핸들이 유효한지 확인합니다 */
	bool IsValid() const { return ExtensionPointId != INDEX_NONE; }

	/** 핸들을 초기화합니다 */
	void Reset() { ExtensionPointId = INDEX_NONE; }

	bool operator==(const FUIExtensionPointHandle& Other) const
	{
		return ExtensionPointId == Other.ExtensionPointId;
	}

	bool operator!=(const FUIExtensionPointHandle& Other) const
	{
		return !(*this == Other);
	}

	friend uint32 GetTypeHash(const FUIExtensionPointHandle& Handle)
	{
		return GetTypeHash(Handle.ExtensionPointId);
	}

private:
	FUIExtensionPointHandle(int32 InId) : ExtensionPointId(InId) {}

	/** 확장 포인트 고유 ID */
	int32 ExtensionPointId = INDEX_NONE;
};

//-----------------------------------------------------------------------------
// FUIExtensionHandle
//-----------------------------------------------------------------------------

/**
 * 확장 핸들 (공급 측)
 *
 * UI 콘텐츠를 등록할 때 반환되며, 등록 해제에 사용됩니다.
 * 예: GameFeature에서 "이 위젯을 HUD에 넣어달라"고 등록한 콘텐츠의 핸들
 */
USTRUCT(BlueprintType)
struct COMMONUIEXTENSION_API FUIExtensionHandle
{
	GENERATED_BODY()

	friend class UUIExtensionSubsystem;

public:
	FUIExtensionHandle() = default;

	/** 핸들이 유효한지 확인합니다 */
	bool IsValid() const { return ExtensionId != INDEX_NONE; }

	/** 핸들을 초기화합니다 */
	void Reset() { ExtensionId = INDEX_NONE; }

	bool operator==(const FUIExtensionHandle& Other) const
	{
		return ExtensionId == Other.ExtensionId;
	}

	bool operator!=(const FUIExtensionHandle& Other) const
	{
		return !(*this == Other);
	}

	friend uint32 GetTypeHash(const FUIExtensionHandle& Handle)
	{
		return GetTypeHash(Handle.ExtensionId);
	}

private:
	FUIExtensionHandle(int32 InId) : ExtensionId(InId) {}

	/** 확장 고유 ID */
	int32 ExtensionId = INDEX_NONE;
};

//-----------------------------------------------------------------------------
// FUIExtensionRequest
//-----------------------------------------------------------------------------

/**
 * 확장 요청 데이터 (장착 요청)
 *
 * 매칭 발생 시 확장 포인트(수신 측)에 전달되는 데이터입니다.
 * 확장 포인트는 이 요청을 받아 실제 위젯 인스턴스를 생성합니다.
 */
USTRUCT(BlueprintType)
struct COMMONUIEXTENSION_API FUIExtensionRequest
{
	GENERATED_BODY()

public:
	/** 확장 핸들 */
	UPROPERTY(BlueprintReadOnly, Category = "UI Extension")
	FUIExtensionHandle ExtensionHandle;

	/** 대상 확장 포인트 태그 */
	UPROPERTY(BlueprintReadOnly, Category = "UI Extension")
	FGameplayTag ExtensionPointTag;

	/** 우선순위 (높을수록 먼저 처리) */
	UPROPERTY(BlueprintReadOnly, Category = "UI Extension")
	int32 Priority = 0;

	/** 확장 데이터 (위젯 클래스 또는 커스텀 데이터) */
	UPROPERTY(BlueprintReadOnly, Category = "UI Extension")
	TObjectPtr<UObject> Data;

	/** 컨텍스트 오브젝트 (플레이어 등) */
	UPROPERTY(BlueprintReadOnly, Category = "UI Extension")
	TObjectPtr<UObject> ContextObject;
};

//-----------------------------------------------------------------------------
// EUIExtensionAction
//-----------------------------------------------------------------------------

/** 확장 포인트 액션 */
UENUM(BlueprintType)
enum class EUIExtensionAction : uint8
{
	/** 확장이 추가됨 */
	Added,

	/** 확장이 제거됨 */
	Removed
};

/** 확장 포인트 콜백 델리게이트 */
DECLARE_DELEGATE_TwoParams(FExtendExtensionPointDelegate, EUIExtensionAction /*Action*/, const FUIExtensionRequest& /*Request*/);
