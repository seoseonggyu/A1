// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ItemFragmentNetState.h"
#include "ItemFragmentNetStateHandle.generated.h"

/**
 * FItemFragmentNetState를 담는 다형성 컨테이너
 *
 * TSharedPtr로 다형성을 지원하며, TPolymorphicStructNetSerializerImpl을 통해
 * 효율적인 네트워크 직렬화를 합니다.
 */
USTRUCT(BlueprintType)
struct COMMONGAME_API FItemFragmentNetStateHandle
{
	GENERATED_BODY()

public:
	FItemFragmentNetStateHandle() = default;

	template<typename T>
	explicit FItemFragmentNetStateHandle(TSharedPtr<T> InData)
		: Data(StaticCastSharedPtr<FItemFragmentNetState>(InData))
	{
	}

	/** 복사 생성자 (Deep Copy) */
	FItemFragmentNetStateHandle(const FItemFragmentNetStateHandle& Other);

	/** 대입 연산자 (Deep Copy) */
	FItemFragmentNetStateHandle& operator=(const FItemFragmentNetStateHandle& Other);

	/** 이동 생성자 */
	FItemFragmentNetStateHandle(FItemFragmentNetStateHandle&& Other) noexcept = default;

	/** 이동 대입 연산자 */
	FItemFragmentNetStateHandle& operator=(FItemFragmentNetStateHandle&& Other) noexcept = default;

public:
	/** 유효한 데이터가 있는지 확인합니다 */
	bool IsValid() const { return Data.IsValid(); }

	/** 데이터 포인터를 반환합니다 */
	FItemFragmentNetState* Get() const { return Data.Get(); }

	/** 지정한 타입으로 캐스팅하여 반환합니다 */
	template<typename T>
	T* Get() const { return static_cast<T*>(Data.Get()); }

	/** 실제 UScriptStruct 타입을 반환합니다 */
	UScriptStruct* GetScriptStruct() const { return Data.IsValid() ? Data->GetScriptStruct() : nullptr; }

	/** 데이터를 초기화합니다 */
	void Reset() { Data.Reset(); }

	/** 비교 연산자 (네트워크 델타 비교에 사용) */
	bool operator==(const FItemFragmentNetStateHandle& Other) const;
	bool operator!=(const FItemFragmentNetStateHandle& Other) const { return !(*this == Other); }

public:
	/** NetState 데이터 (TSharedPtr로 다형성 지원) */
	TSharedPtr<FItemFragmentNetState> Data;
};

template<>
struct TStructOpsTypeTraits<FItemFragmentNetStateHandle> : public TStructOpsTypeTraitsBase2<FItemFragmentNetStateHandle>
{
	enum
	{
		WithCopy = true, // 복사 연산자 호출
		WithIdenticalViaEquality = true // operator ==
	};
};
