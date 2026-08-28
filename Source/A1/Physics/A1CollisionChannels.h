#pragma once

#include "Engine/EngineTypes.h"

//-----------------------------------------------------------------------------
// A1 커스텀 콜리전 채널
//
// DefaultEngine.ini의 [/Script/Engine.CollisionProfile]와 값이 일치해야 한다.
// 채널을 추가/이동할 경우 .ini의 ECC_GameTraceChannelN 번호도 함께 맞춘다.
//-----------------------------------------------------------------------------

/** 마우스 커서로 상호작용 대상을 탐지하는 트레이스 채널 (로컬 전용). */
#define A1_TraceChannel_Interaction ECC_GameTraceChannel1

/**
 * 투사체(AA1Projectile)의 오브젝트 타입. 기본 응답은 Block(벽 등 월드 지오메트리는 막음)이고,
 * 캐릭터 캡슐만 Overlap으로 재정의(AA1Character 생성자)해 피격 판정이 가능하게 한다.
 * ECC_WorldDynamic을 그대로 쓰면 Pawn 프로필이 기본적으로 WorldDynamic을 Block하기 때문에
 * 투사체가 캐릭터에 닿아도 오버랩(BeginOverlap)이 아니라 블로킹으로 처리되어 피격 이벤트가 전혀
 * 발생하지 않는 문제가 있었다.
 */
#define A1_TraceChannel_Projectile ECC_GameTraceChannel2
