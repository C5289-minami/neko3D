#include "BossIdleState.h"
#include "Boss.h"

BossIdleState::BossIdleState(BossStateMachine& stateMachine, Boss& boss)
    : BossBaseState(stateMachine, boss)
{
}

void BossIdleState::Enter()
{
    boss_.StopMove();
}

void BossIdleState::Tick(float)
{
}

void BossIdleState::Exit()
{
}
