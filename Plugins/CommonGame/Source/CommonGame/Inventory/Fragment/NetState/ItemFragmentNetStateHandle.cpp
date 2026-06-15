// Copyright Epic Games, Inc. All Rights Reserved.

#include "ItemFragmentNetStateHandle.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(ItemFragmentNetStateHandle)

DEFINE_LOG_CATEGORY_STATIC(LogFragmentNetStateHandle, Log, All);

FItemFragmentNetStateHandle::FItemFragmentNetStateHandle(const FItemFragmentNetStateHandle& Other)
{
	if (!Other.IsValid())
	{
		return;
	}

	UScriptStruct* ScriptStruct = Other.GetScriptStruct();
	if (!ScriptStruct)
	{
		UE_LOG(LogFragmentNetStateHandle, Warning, TEXT("NetState가 유효하지만 GetScriptStruct()가 nullptr입니다"));
		return;
	}

	// 새 메모리 할당 및 구조체 초기화
	FItemFragmentNetState* NewData = static_cast<FItemFragmentNetState*>(
		FMemory::Malloc(ScriptStruct->GetStructureSize(), ScriptStruct->GetMinAlignment()));
	ScriptStruct->InitializeStruct(NewData);

	// 데이터 복사
	ScriptStruct->CopyScriptStruct(NewData, Other.Get());

	// TSharedPtr에 할당 (커스텀 삭제자 사용)
	Data = TSharedPtr<FItemFragmentNetState>(NewData, [ScriptStruct](FItemFragmentNetState* Ptr)
	{
		if (Ptr)
		{
			ScriptStruct->DestroyStruct(Ptr);
			FMemory::Free(Ptr);
		}
	});
}

FItemFragmentNetStateHandle& FItemFragmentNetStateHandle::operator=(const FItemFragmentNetStateHandle& Other)
{
	if (this == &Other)
	{
		return *this;
	}

	if (!Other.IsValid())
	{
		Data.Reset();
		return *this;
	}

	UScriptStruct* ScriptStruct = Other.GetScriptStruct();
	if (!ScriptStruct)
	{
		UE_LOG(LogFragmentNetStateHandle, Warning, TEXT("NetState가 유효하지만 GetScriptStruct()가 nullptr입니다"));
		Data.Reset();
		return *this;
	}

	// 새 메모리 할당 및 구조체 초기화
	FItemFragmentNetState* NewData = static_cast<FItemFragmentNetState*>(
		FMemory::Malloc(ScriptStruct->GetStructureSize(), ScriptStruct->GetMinAlignment()));
	ScriptStruct->InitializeStruct(NewData);

	// 데이터 복사
	ScriptStruct->CopyScriptStruct(NewData, Other.Get());

	// TSharedPtr에 할당 (커스텀 삭제자 사용)
	Data = TSharedPtr<FItemFragmentNetState>(NewData, [ScriptStruct](FItemFragmentNetState* Ptr)
	{
		if (Ptr)
		{
			ScriptStruct->DestroyStruct(Ptr);
			FMemory::Free(Ptr);
		}
	});

	return *this;
}

bool FItemFragmentNetStateHandle::operator==(const FItemFragmentNetStateHandle& Other) const
{
	UScriptStruct* ScriptStruct = GetScriptStruct();
	UScriptStruct* OtherScriptStruct = Other.GetScriptStruct();

	// 둘 다 null이면 같음
	if (!ScriptStruct && !OtherScriptStruct)
	{
		return true;
	}

	// 타입이 다르면 다름 (하나만 null인 경우 포함)
	if (ScriptStruct != OtherScriptStruct)
	{
		return false;
	}

	// 같은 타입이면 구조체 내용 비교
	return ScriptStruct->CompareScriptStruct(Get(), Other.Get(), PPF_None);
}
