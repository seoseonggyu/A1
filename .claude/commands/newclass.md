---
description: 프로젝트 규칙에 맞는 새 C++ 클래스/구조체 생성
argument-hint: <클래스명> <베이스클래스> [배치 경로]
---

`$ARGUMENTS` 기준으로 새 클래스를 만든다.

먼저 확인할 것:
- 배치 위치: 게임 고유 로직이면 `Source/A1/<도메인>/`, 재사용 가능한 프레임워크면 `Plugins/CommonGame/Source/CommonGame/<도메인>/`
- 새 클래스 대신 **Fragment / ActorExtension Execute** 로 표현할 수 있는지 (가능하면 그쪽을 우선)

생성 규칙:
- 파일명 = 클래스명(접두사 제외). 게임 모듈 클래스는 `A1` 접두사 (`AA1Foo`, `UA1Foo`)
- 헤더: `#pragma once` → 엔진/외부 include → 전방 선언 → **`"<파일명>.generated.h"` 는 마지막**
- API 매크로: `A1_API` / `COMMONGAME_API` / `COMMONUIEXTENSION_API`
- 클래스 위에 역할·동작 방식을 설명하는 한국어 `/** */` 블록 주석
- 전용 로그 카테고리: 헤더 `DECLARE_LOG_CATEGORY_EXTERN(<클래스명>Log, Log, All);` / cpp `DEFINE_LOG_CATEGORY(<클래스명>Log);`
- 멤버 포인터는 `TObjectPtr<T>`, 생성자는 `const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()` 시그니처
- 복제 프로퍼티가 있으면 `GetLifetimeReplicatedProps` 오버라이드까지 함께 작성
- 인코딩 UTF-8 BOM, 개행 CRLF, 들여쓰기 탭, 중괄호 Allman

마지막에 프로젝트 파일 재생성이 필요하다고 안내한다.
