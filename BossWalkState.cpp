#include "BossWalkState.h"
#include "Boss.h"

BossWalkState::BossWalkState(BossStateMachine& stateMachine, Boss& boss)
    : BossBaseState(stateMachine, boss)
{
}

void BossWalkState::Enter()
{
}

void BossWalkState::Tick(float)
{
}

void BossWalkState::Exit()
{
    boss_.StopMove();
}
