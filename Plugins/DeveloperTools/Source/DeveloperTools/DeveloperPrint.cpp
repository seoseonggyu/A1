#include "DeveloperPrint.h"
#include "DeveloperStatics.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/Object.h"

DEFINE_LOG_CATEGORY_STATIC(DeveloperPrintLog, Log, All);

void FDeveloperPrint::PrintString(const FString& InString, const FColor Color, const float Duration, const UObject* WorldContext, const int32 Key)
{
	FString FinalString = InString;

	// WorldContext가 있으면 NetMode 접두사를 붙여 서버/클라 구분을 쉽게 한다.
	if (WorldContext)
	{
		if (const UWorld* World = WorldContext->GetWorld())
		{
			FinalString = FString::Printf(TEXT("%s %s"), FDeveloperStatics::GetNetModeString(World), *InString);
		}
	}

	// 출력 로그에는 항상 남긴다.
	UE_LOG(DeveloperPrintLog, Log, TEXT("%s"), *FinalString);

	// 화면 출력은 개발용이므로 셰이핑 빌드에서는 제외한다.
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(Key, Duration, Color, FinalString);
	}
#endif
}
