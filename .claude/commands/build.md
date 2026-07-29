---
description: A1Editor 타깃 컴파일 (필요 시 프로젝트 파일 재생성)
argument-hint: [regen]
---

A1 프로젝트를 빌드한다.

1. `C:\Program Files\UE_5.8` 와 `D:\UE_5.8` 중 실제 존재하는 엔진 경로를 확인한다.
2. `$ARGUMENTS` 에 `regen` 이 있거나 이번 세션에서 `.h`/`.cpp` 파일을 **새로 추가**했다면 먼저 프로젝트 파일을 재생성한다:
   `Build.bat -projectfiles -project="<프로젝트경로>\A1.uproject" -game -rocket -progress`
3. 컴파일:
   `Build.bat A1Editor Win64 Development -Project="<프로젝트경로>\A1.uproject" -WaitMutex`
4. 실패하면 **첫 번째 에러**부터 원인을 분석해 보고한다. 흔한 원인:
   - `.generated.h` 를 마지막 include로 넣지 않음
   - Build.cs 의존 모듈 누락
   - `GetLifetimeReplicatedProps` / `DEFINE_LOG_CATEGORY` 누락
5. 에디터가 실행 중이면 파일 잠금으로 실패한다 — 그 경우 에디터를 닫거나 Live Coding을 쓰라고 안내한다.
