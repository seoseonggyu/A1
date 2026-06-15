// Copyright Epic Games, Inc. All Rights Reserved.

#include "FragmentNetStateHandleNetSerializer.h"
#include "Inventory/Fragment/NetState/ItemFragmentNetStateHandle.h"
#include "Iris/Serialization/NetSerializerDelegates.h"
#include "Iris/Serialization/PolymorphicNetSerializerImpl.h"
#include "Iris/ReplicationState/PropertyNetSerializerInfoRegistry.h"

namespace UE::Net
{

//-----------------------------------------------------------------------------
// FItemFragmentNetStateHandleAccessor
//-----------------------------------------------------------------------------

/** TSharedPtr 접근자 (TPolymorphicStructNetSerializerImpl 요구사항) */
struct FItemFragmentNetStateHandleAccessor
{
	static TSharedPtr<FItemFragmentNetState>& GetItem(FItemFragmentNetStateHandle& Handle)
	{
		return Handle.Data;
	}
};

//-----------------------------------------------------------------------------
// FFragmentNetStateHandleNetSerializer
//-----------------------------------------------------------------------------

/**
 * FItemFragmentNetStateHandle용 NetSerializer
 *
 * TPolymorphicStructNetSerializerImpl을 상속하여 다형성 직렬화를 지원합니다.
 */
struct FFragmentNetStateHandleNetSerializer
	: TPolymorphicStructNetSerializerImpl<FItemFragmentNetStateHandle, FItemFragmentNetState, FItemFragmentNetStateHandleAccessor::GetItem>
{
public:
	typedef TPolymorphicStructNetSerializerImpl<FItemFragmentNetStateHandle, FItemFragmentNetState, FItemFragmentNetStateHandleAccessor::GetItem> InternalSerializerType;
	typedef FPolymorphicStructNetSerializerConfig ConfigType;

	static const uint32 Version = 0;
	static const ConfigType DefaultConfig;

	/** 타입 캐시 초기화 */
	static void InitTypeCache()
	{
		if (bIsPostFreezeCalled)
		{
			InternalSerializerType::InitTypeCache<FFragmentNetStateHandleNetSerializer>();
		}
	}

private:
	//-----------------------------------------------------------------------------
	// 레지스트리 델리게이트
	//-----------------------------------------------------------------------------

	class FNetSerializerRegistryDelegates final : private UE::Net::FNetSerializerRegistryDelegates
	{
	public:
		FNetSerializerRegistryDelegates();
		virtual ~FNetSerializerRegistryDelegates() override;

	private:
		virtual void OnPreFreezeNetSerializerRegistry() override;
		virtual void OnPostFreezeNetSerializerRegistry() override;
		virtual void OnLoadedModulesUpdated() override;
	};

	static FNetSerializerRegistryDelegates NetSerializerRegistryDelegates;
	static bool bIsPostFreezeCalled;
};

//-----------------------------------------------------------------------------
// NetSerializer 등록
//-----------------------------------------------------------------------------

UE_NET_IMPLEMENT_SERIALIZER(FFragmentNetStateHandleNetSerializer);

const FFragmentNetStateHandleNetSerializer::ConfigType FFragmentNetStateHandleNetSerializer::DefaultConfig;
FFragmentNetStateHandleNetSerializer::FNetSerializerRegistryDelegates FFragmentNetStateHandleNetSerializer::NetSerializerRegistryDelegates;
bool FFragmentNetStateHandleNetSerializer::bIsPostFreezeCalled = false;

// 레지스트리에 "ItemFragmentNetStateHandle" 이름으로 등록
static const FName PropertyNetSerializerRegistry_NAME_ItemFragmentNetStateHandle(TEXT("ItemFragmentNetStateHandle"));
UE_NET_IMPLEMENT_NAMED_STRUCT_NETSERIALIZER_INFO(PropertyNetSerializerRegistry_NAME_ItemFragmentNetStateHandle, FFragmentNetStateHandleNetSerializer);

FFragmentNetStateHandleNetSerializer::FNetSerializerRegistryDelegates::FNetSerializerRegistryDelegates()
	: UE::Net::FNetSerializerRegistryDelegates(EFlags::ShouldBindLoadedModulesUpdatedDelegate)
{
}

FFragmentNetStateHandleNetSerializer::FNetSerializerRegistryDelegates::~FNetSerializerRegistryDelegates()
{
	UE_NET_UNREGISTER_NETSERIALIZER_INFO(PropertyNetSerializerRegistry_NAME_ItemFragmentNetStateHandle);
}

void FFragmentNetStateHandleNetSerializer::FNetSerializerRegistryDelegates::OnPreFreezeNetSerializerRegistry()
{
	UE_NET_REGISTER_NETSERIALIZER_INFO(PropertyNetSerializerRegistry_NAME_ItemFragmentNetStateHandle);
}

void FFragmentNetStateHandleNetSerializer::FNetSerializerRegistryDelegates::OnPostFreezeNetSerializerRegistry()
{
	bIsPostFreezeCalled = true;
}

void FFragmentNetStateHandleNetSerializer::FNetSerializerRegistryDelegates::OnLoadedModulesUpdated()
{
	InitFragmentNetStateHandleNetSerializerTypeCache();
}

}

//-----------------------------------------------------------------------------
// 글로벌 초기화 함수
//-----------------------------------------------------------------------------

void InitFragmentNetStateHandleNetSerializerTypeCache()
{
	UE::Net::FFragmentNetStateHandleNetSerializer::InitTypeCache();
}
