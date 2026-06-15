// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Iris/Serialization/NetSerializer.h"

namespace UE::Net
{

UE_NET_DECLARE_SERIALIZER(FFragmentNetStateHandleNetSerializer, COMMONGAME_API);

}

/** 타입 캐시 초기화 함수 (모듈 시작 시 호출) */
COMMONGAME_API void InitFragmentNetStateHandleNetSerializerTypeCache();
