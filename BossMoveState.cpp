#include "BossMoveState.h"
#include "Boss.h"

BossMoveState::BossMoveState(BossStateMachine& stateMachine, Boss& boss)
    : BossBaseState(stateMachine, boss)
{
}

void BossMoveState::Enter()
{
}

void BossMoveState::Tick(float)
{
}

void BossMoveState::Exit()
{
    boss_.StopMove();
}
