// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Soft Reference를 Client/Server 번들별로 수집하는 유틸리티
 *
 * UPROPERTY의 AssetBundles 메타데이터에 따라 분류:
 * - "Client" → OutClientPaths에만
 * - "Server" → OutServerPaths에만
 * - "All" → 둘 다
 * - 미지정 → 수집하지 않음
 *
 * TArray, TMap, TInstancedStruct 내부까지 재귀 탐색합니다.
 *
 * 예시: UPROPERTY(meta = (AssetBundles = "Client"))
 *       UPROPERTY(meta = (AssetBundles = "Server"))
 *       UPROPERTY(meta = (AssetBundles = "All"))
 */
struct COMMONGAME_API FSoftPathCollector
{
public:
	/**
	 * 객체의 모든 Soft Reference를 Client/Server별로 수집합니다
	 * @param Struct 탐색할 UStruct (UClass 포함)
	 * @param Container 실제 데이터 포인터
	 * @param OutClientPaths Client 번들에 등록할 경로들
	 * @param OutServerPaths Server 번들에 등록할 경로들
	 */
	static void CollectSoftObjectPaths(const UStruct* Struct, const void* Container, TArray<FSoftObjectPath>& OutClientPaths, TArray<FSoftObjectPath>& OutServerPaths);

private:
	/** 재귀 탐색 내부 구현 */
	static void CollectSoftObjectPathsInternal(const UStruct* Struct, const void* Container, TArray<FSoftObjectPath>& OutClientPaths, TArray<FSoftObjectPath>& OutServerPaths, TSet<const void*>& VisitedContainers);

	/** 단일 프로퍼티에서 Soft Reference 추출 (배열/맵 요소용, 부모 메타데이터 전달) */
	static void CollectFromProperty(const FProperty* Property, const void* ValuePtr, bool bParentClient, bool bParentServer, TArray<FSoftObjectPath>& OutClientPaths, TArray<FSoftObjectPath>& OutServerPaths, TSet<const void*>& VisitedContainers);

	/** 프로퍼티의 AssetBundles 메타데이터 파싱 */
	static void ParseBundleMetadata(const FProperty* Property, bool& bOutClient, bool& bOutServer);

	/** Soft Reference 경로를 적절한 배열에 추가 */
	static void AddPathToBundles(const FSoftObjectPath& Path, bool bClient, bool bServer, TArray<FSoftObjectPath>& OutClientPaths, TArray<FSoftObjectPath>& OutServerPaths);
};
