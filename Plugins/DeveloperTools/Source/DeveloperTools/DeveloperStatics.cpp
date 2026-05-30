#include "DeveloperStatics.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"

//-----------------------------------------------------------------------------
// 네트워크 디버깅 문자열
//-----------------------------------------------------------------------------

const TCHAR* FDeveloperStatics::GetNetRoleString(const AActor* Actor)
{
	if (!Actor)
	{
		return TEXT("[NULL]");
	}
	return Actor->HasAuthority() ? TEXT("[서버]") : TEXT("[클라]");
}

const TCHAR* FDeveloperStatics::GetNetModeString(const UWorld* World)
{
	if (!World)
	{
		return TEXT("[NULL]");
	}

	switch (World->GetNetMode())
	{
	case NM_Standalone:
		return TEXT("[스탠드얼론]");
	case NM_DedicatedServer:
		return TEXT("[데디 서버]");
	case NM_ListenServer:
		return TEXT("[리슨 서버]");
	case NM_Client:
		return TEXT("[클라이언트]");
	default:
		return TEXT("[알 수 없음]");
	}
}

//-----------------------------------------------------------------------------
// PIE Authority 접근
//-----------------------------------------------------------------------------

UWorld* FDeveloperStatics::FindPlayInEditorAuthorityWorld()
{
    check(GEngine);

    UWorld* ServerWorld = nullptr;

#if WITH_EDITOR
    for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
    {
        if (WorldContext.WorldType == EWorldType::PIE)
        {
            if (UWorld* TestWorld = WorldContext.World())
            {
                if (WorldContext.RunAsDedicated)
                {
                    ServerWorld = TestWorld;
                    break;
                }
                else if (ServerWorld == nullptr)
                {
                    ServerWorld = TestWorld;
                }
                else
                {
                    if (TestWorld->GetNetMode() < ServerWorld->GetNetMode())
                    {
                        ServerWorld = TestWorld;
                    }
                }
            }
        }
    }
#endif

    return ServerWorld;
}

APlayerController* FDeveloperStatics::FindPlayInEditorAuthorityPlayerController(APlayerController* ClientController)
{
    if (ClientController == nullptr)
    {
        return nullptr;
    }

#if WITH_EDITOR
    UWorld* ServerWorld = FindPlayInEditorAuthorityWorld();
    if (ServerWorld == nullptr)
    {
        return nullptr;
    }

    for (FConstPlayerControllerIterator Iterator = ServerWorld->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        APlayerController* ServerController = Iterator->Get();
        if (ServerController == nullptr || ServerController->PlayerState == nullptr || ClientController->PlayerState == nullptr)
        {
            continue;
        }

        const FUniqueNetIdRepl& ServerPlayerUniqueId = ServerController->PlayerState->GetUniqueId();
        const FUniqueNetIdRepl& ClientPlayerUniqueId = ClientController->PlayerState->GetUniqueId();

        if (ServerPlayerUniqueId == ClientPlayerUniqueId)
        {
            return ServerController;
        }
    }
#endif

    return nullptr;
}

APawn* FDeveloperStatics::FindPlayInEditorAuthorityPawn(APawn* ClientPawn)
{
    if (ClientPawn == nullptr)
    {
        return nullptr;
    }

#if WITH_EDITOR
    APlayerController* ClientController = Cast<APlayerController>(ClientPawn->GetController());
    if (ClientController == nullptr)
    {
        return nullptr;
    }

    APlayerController* ServerController = FindPlayInEditorAuthorityPlayerController(ClientController);
    if (ServerController == nullptr)
    {
        return nullptr;
    }

    return ServerController->GetPawn();
#else
    return nullptr;
#endif
}
