#pragma once

#include "CoreMinimal.h"

class AActor;
class UWorld;

/**
 * 에디터 전용 네트워크 디버깅 유틸리티
 *
 * PIE 환경에서 클라이언트 인스턴스에서 서버 인스턴스에 접근하여
 * 클라이언트/서버 간 값 차이를 쉽게 비교할 수 있게 해주는 함수들을 제공합니다.
 */
class DEVELOPERTOOLS_API FDeveloperStatics
{
public:
	//-----------------------------------------------------------------------------
	// 네트워크 디버깅 문자열
	//-----------------------------------------------------------------------------

	/** Actor의 Authority 여부에 따른 문자열 반환 ("[서버]" 또는 "[클라]") */
	static const TCHAR* GetNetRoleString(const AActor* Actor);

	/** World의 NetMode에 따른 문자열 반환 */
	static const TCHAR* GetNetModeString(const UWorld* World);

	//-----------------------------------------------------------------------------
	// PIE Authority 접근
	//-----------------------------------------------------------------------------

    /**
     * PIE 환경에서 서버 월드를 찾아 반환
     * @return 서버 월드. 에디터가 아니거나 찾지 못한 경우 nullptr
     */
    static UWorld* FindPlayInEditorAuthorityWorld();

    /**
     * 클라이언트의 PlayerController에 대응하는 서버의 PlayerController를 반환
     * @param ClientController 클라이언트의 PlayerController
     * @return 서버의 PlayerController. 찾지 못한 경우 nullptr
     */
    static APlayerController* FindPlayInEditorAuthorityPlayerController(APlayerController* ClientController);

    /**
     * 클라이언트의 Pawn에 대응하는 서버의 Pawn을 반환
     * @param ClientPawn 클라이언트의 Pawn
     * @return 서버의 Pawn. 찾지 못한 경우 nullptr
     */
    static APawn* FindPlayInEditorAuthorityPawn(APawn* ClientPawn);
};
