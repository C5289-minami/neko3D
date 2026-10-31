#include "BossIdleState.h"
#include "Boss.h"
#include "Consts.h"

#include "DxPlus/Debug.h"

BossIdleState::BossIdleState(BossStateMachine& stateMachine, Boss& boss)
    : BossBaseState(stateMachine, boss)
{
}

void BossIdleState::Enter()
{
    boss_.StopMove();
    boss_.SetAnimation(BossAnimType::Idle);
}

void BossIdleState::Tick(float deltaTime)
{
    boss_.UpdateAnimation(deltaTime);
    DxPlus::Debug::SetString(L"CurrentBossState:Idle");
}

void BossIdleState::Exit()
{
}
