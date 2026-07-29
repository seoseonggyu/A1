#pragma once

#include "CoreMinimal.h"

class UObject;

/**
 * 화면 디버그 출력 유틸리티
 *
 * BP의 PrintString 노드처럼 C++ 코드에서 화면과 출력 로그에 문자열을 띄웁니다.
 * 화면 출력은 셰이핑 빌드에서 컴파일되지 않으며, 로그 출력만 남습니다.
 *
 * 사용 예:
 * @code
 * #include "DeveloperPrint.h"
 *
 * // 가장 단순한 형태 (기본 색·2초)
 * FDeveloperPrint::PrintString(TEXT("공격 시작"));
 *
 * // 색·시간 지정 + WorldContext로 [서버]/[클라] 접두사 자동 부착
 * FDeveloperPrint::PrintString(TEXT("회전 잠금 해제"), FColor::Yellow, 3.0f, this);
 *
 * // 서식 문자열 (FString::Printf 축약)
 * FDeveloperPrint::PrintStringf(TEXT("ComboIndex: %d"), ComboIndex);
 *
 * // 같은 Key로 매 프레임 호출하면 한 줄이 갱신됨 (스팸 방지)
 * FDeveloperPrint::PrintString(TEXT("Tick 중"), FColor::Green, 0.f, this, 1);
 * @endcode
 */
class DEVELOPERTOOLS_API FDeveloperPrint
{
public:
	//-----------------------------------------------------------------------------
	// 화면 출력
	//-----------------------------------------------------------------------------

	/**
	 * 문자열을 화면과 출력 로그에 표시한다. (BP PrintString 대응)
	 * @param InString      출력할 문자열
	 * @param Color         화면에 표시할 색상
	 * @param Duration      화면에 유지되는 시간(초)
	 * @param WorldContext  넘기면 "[서버]/[클라]" 같은 NetMode 접두사를 앞에 붙인다. (nullptr이면 생략)
	 * @param Key           같은 Key로 다시 호출하면 기존 메시지를 덮어쓴다. INDEX_NONE이면 항상 새 줄로 추가.
	 */
	static void PrintString(const FString& InString, const FColor Color = FColor::Cyan, const float Duration = 2.0f, const UObject* WorldContext = nullptr, const int32 Key = INDEX_NONE);

	/** 서식 문자열 버전. PrintString(FString::Printf(...)) 축약. */
	template <typename FmtType, typename... Types>
	static void PrintStringf(const FmtType& Fmt, Types... Args)
	{
		PrintString(FString::Printf(Fmt, Args...));
	}
};
