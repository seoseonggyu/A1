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
