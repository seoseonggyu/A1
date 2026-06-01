// Copyright Epic Games, Inc. All Rights Reserved.

#include "Utility/SoftPathCollector.h"
#include "StructUtils/InstancedStruct.h"

void FSoftPathCollector::CollectSoftObjectPaths(const UStruct* Struct, const void* Container, TArray<FSoftObjectPath>& OutClientPaths, TArray<FSoftObjectPath>& OutServerPaths)
{
#if WITH_EDITOR
	// 에디터: 정적 풀로 메모리 재사용
	static TSet<const void*> VisitedContainers = []()
		{
			TSet<const void*> Set;
			Set.Reserve(10000);
			return Set;
		}();
#else
	// 런타임: 필요할 때만 할당
	TSet<const void*> VisitedContainers;
#endif

	CollectSoftObjectPathsInternal(Struct, Container, OutClientPaths, OutServerPaths, VisitedContainers);
	VisitedContainers.Reset();
}

void FSoftPathCollector::CollectSoftObjectPathsInternal(const UStruct* Struct, const void* Container, TArray<FSoftObjectPath>& OutClientPaths, TArray<FSoftObjectPath>& OutServerPaths, TSet<const void*>& VisitedContainers)
{
	// 순환 참조 방지
	if (VisitedContainers.Contains(Container))
	{
		return;
	}
	VisitedContainers.Add(Container);

	for (TFieldIterator<FProperty> PropIt(Struct); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;
		const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Container);

		// 메타데이터에서 Client/Server 플래그 파싱
		bool bClient, bServer;
		ParseBundleMetadata(Property, bClient, bServer);

		// TSoftObjectPtr<T>
		if (const FSoftObjectProperty* SoftObjectProp = CastField<FSoftObjectProperty>(Property))
		{
			const FSoftObjectPtr& SoftPtr = SoftObjectProp->GetPropertyValue(ValuePtr);
			if (!SoftPtr.IsNull())
			{
				AddPathToBundles(SoftPtr.ToSoftObjectPath(), bClient, bServer, OutClientPaths, OutServerPaths);
			}
		}
		// TSoftClassPtr<T>
		else if (const FSoftClassProperty* SoftClassProp = CastField<FSoftClassProperty>(Property))
		{
			const FSoftObjectPtr& SoftPtr = SoftClassProp->GetPropertyValue(ValuePtr);
			if (!SoftPtr.IsNull())
			{
				AddPathToBundles(SoftPtr.ToSoftObjectPath(), bClient, bServer, OutClientPaths, OutServerPaths);
			}
		}
		// FSoftObjectPath / FSoftClassPath 구조체 직접 사용
		else if (const FStructProperty* StructProp = CastField<FStructProperty>(Property))
		{
			// FSoftObjectPath와 FSoftClassPath는 바이너리 동일
			if (StructProp->Struct == TBaseStructure<FSoftObjectPath>::Get() ||
				StructProp->Struct == TBaseStructure<FSoftClassPath>::Get())
			{
				const FSoftObjectPath* PathPtr = static_cast<const FSoftObjectPath*>(ValuePtr);
				if (PathPtr && !PathPtr->IsNull())
				{
					AddPathToBundles(*PathPtr, bClient, bServer, OutClientPaths, OutServerPaths);
				}

				// 내부 문자열 프로퍼티 탐색 불필요
				continue;
			}

			// TInstancedStruct 처리
			if (StructProp->Struct->IsChildOf(FInstancedStruct::StaticStruct()))
			{
				const FInstancedStruct* InstancedStruct = static_cast<const FInstancedStruct*>(ValuePtr);
				if (const UScriptStruct* ScriptStruct = InstancedStruct->GetScriptStruct())
				{
					if (const void* Memory = InstancedStruct->GetMemory())
					{
						CollectSoftObjectPathsInternal(ScriptStruct, Memory, OutClientPaths, OutServerPaths, VisitedContainers);
					}
				}
			}
			else
			{
				// 일반 구조체 재귀 탐색
				CollectSoftObjectPathsInternal(StructProp->Struct, ValuePtr, OutClientPaths, OutServerPaths, VisitedContainers);
			}
		}
		// UObject 프로퍼티 (TObjectPtr<T>, T*)
		else if (const FObjectPropertyBase* ObjectProp = CastField<FObjectPropertyBase>(Property))
		{
			if (UObject* Object = ObjectProp->GetObjectPropertyValue(ValuePtr))
			{
				CollectSoftObjectPathsInternal(Object->GetClass(), Object, OutClientPaths, OutServerPaths, VisitedContainers);
			}
		}
		// TArray - 배열 프로퍼티의 메타데이터를 요소에 전달
		else if (const FArrayProperty* ArrayProp = CastField<FArrayProperty>(Property))
		{
			FScriptArrayHelper ArrayHelper(ArrayProp, ValuePtr);
			for (int32 i = 0; i < ArrayHelper.Num(); ++i)
			{
				CollectFromProperty(ArrayProp->Inner, ArrayHelper.GetRawPtr(i), bClient, bServer, OutClientPaths, OutServerPaths, VisitedContainers);
			}
		}
		// TMap - 맵 프로퍼티의 메타데이터를 Key/Value에 전달
		else if (const FMapProperty* MapProp = CastField<FMapProperty>(Property))
		{
			FScriptMapHelper MapHelper(MapProp, ValuePtr);
			for (int32 i = 0; i < MapHelper.Num(); ++i)
			{
				CollectFromProperty(MapProp->KeyProp, MapHelper.GetKeyPtr(i), bClient, bServer, OutClientPaths, OutServerPaths, VisitedContainers);
				CollectFromProperty(MapProp->ValueProp, MapHelper.GetValuePtr(i), bClient, bServer, OutClientPaths, OutServerPaths, VisitedContainers);
			}
		}
	}
}

void FSoftPathCollector::CollectFromProperty(const FProperty* Property, const void* ValuePtr, bool bParentClient, bool bParentServer, TArray<FSoftObjectPath>& OutClientPaths, TArray<FSoftObjectPath>& OutServerPaths, TSet<const void*>& VisitedContainers)
{
	// TSoftObjectPtr<T> - 부모(배열/맵) 메타데이터 사용
	if (const FSoftObjectProperty* SoftObjectProp = CastField<FSoftObjectProperty>(Property))
	{
		const FSoftObjectPtr& SoftPtr = SoftObjectProp->GetPropertyValue(ValuePtr);
		if (!SoftPtr.IsNull())
		{
			AddPathToBundles(SoftPtr.ToSoftObjectPath(), bParentClient, bParentServer, OutClientPaths, OutServerPaths);
		}
	}
	// TSoftClassPtr<T> - 부모(배열/맵) 메타데이터 사용
	else if (const FSoftClassProperty* SoftClassProp = CastField<FSoftClassProperty>(Property))
	{
		const FSoftObjectPtr& SoftPtr = SoftClassProp->GetPropertyValue(ValuePtr);
		if (!SoftPtr.IsNull())
		{
			AddPathToBundles(SoftPtr.ToSoftObjectPath(), bParentClient, bParentServer, OutClientPaths, OutServerPaths);
		}
	}
	// USTRUCT
	else if (const FStructProperty* StructProp = CastField<FStructProperty>(Property))
	{
		// FSoftObjectPath / FSoftClassPath - 부모(배열/맵) 메타데이터 사용
		if (StructProp->Struct == TBaseStructure<FSoftObjectPath>::Get() ||
			StructProp->Struct == TBaseStructure<FSoftClassPath>::Get())
		{
			const FSoftObjectPath* PathPtr = static_cast<const FSoftObjectPath*>(ValuePtr);
			if (PathPtr && !PathPtr->IsNull())
			{
				AddPathToBundles(*PathPtr, bParentClient, bParentServer, OutClientPaths, OutServerPaths);
			}
			return;
		}

		// TInstancedStruct - 내부 구조체의 개별 프로퍼티 메타데이터 사용
		if (StructProp->Struct->IsChildOf(FInstancedStruct::StaticStruct()))
		{
			const FInstancedStruct* InstancedStruct = static_cast<const FInstancedStruct*>(ValuePtr);
			if (const UScriptStruct* ScriptStruct = InstancedStruct->GetScriptStruct())
			{
				if (const void* Memory = InstancedStruct->GetMemory())
				{
					CollectSoftObjectPathsInternal(ScriptStruct, Memory, OutClientPaths, OutServerPaths, VisitedContainers);
				}
			}
		}
		// 일반 구조체 - 내부 프로퍼티 메타데이터 사용
		else
		{
			CollectSoftObjectPathsInternal(StructProp->Struct, ValuePtr, OutClientPaths, OutServerPaths, VisitedContainers);
		}
	}
	// UObject - 내부 프로퍼티 메타데이터 사용
	else if (const FObjectPropertyBase* ObjectProp = CastField<FObjectPropertyBase>(Property))
	{
		if (UObject* Object = ObjectProp->GetObjectPropertyValue(ValuePtr))
		{
			CollectSoftObjectPathsInternal(Object->GetClass(), Object, OutClientPaths, OutServerPaths, VisitedContainers);
		}
	}
}

void FSoftPathCollector::ParseBundleMetadata(const FProperty* Property, bool& bOutClient, bool& bOutServer)
{
	// 기본값: 둘 다 false (미지정 시 수집하지 않음)
	bOutClient = false;
	bOutServer = false;

	if (!Property)
	{
		return;
	}

#if WITH_EDITORONLY_DATA
	// AssetBundles 메타데이터 확인
	const FString& Bundles = Property->GetMetaData(TEXT("AssetBundles"));
	if (Bundles.IsEmpty())
	{
		return; // 미지정 → 수집 안 함
	}

	TArray<FString> BundleNames;
	Bundles.ParseIntoArray(BundleNames, TEXT(","));

	for (const FString& Name : BundleNames)
	{
		FString Trimmed = Name.TrimStartAndEnd();
		if (Trimmed.Equals(TEXT("Client"), ESearchCase::IgnoreCase))
		{
			bOutClient = true;
		}
		else if (Trimmed.Equals(TEXT("Server"), ESearchCase::IgnoreCase))
		{
			bOutServer = true;
		}
		else if (Trimmed.Equals(TEXT("All"), ESearchCase::IgnoreCase))
		{
			bOutClient = true;
			bOutServer = true;
		}
	}
#endif
}

void FSoftPathCollector::AddPathToBundles(const FSoftObjectPath& Path, bool bClient, bool bServer, TArray<FSoftObjectPath>& OutClientPaths, TArray<FSoftObjectPath>& OutServerPaths)
{
	if (Path.IsNull())
	{
		return;
	}

	if (bClient)
	{
		OutClientPaths.AddUnique(Path);
	}
	if (bServer)
	{
		OutServerPaths.AddUnique(Path);
	}
}
