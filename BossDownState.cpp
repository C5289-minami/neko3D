#include "BossDownState.h"
#include "Boss.h"
#include "BossStateMachine.h"

BossDownState::BossDownState(BossStateMachine& stateMachine, Boss& boss)
    : BossBaseState(stateMachine, boss)
{
}

void BossDownState::Enter()
{
    remainingSeconds_ = DownDurationSeconds;
    boss_.StopMove();
    boss_.SetAnimation(BossAnimType::Damage, false);
}

void BossDownState::Tick(float deltaTime)
{
    boss_.UpdateAnimation(deltaTime);
    remainingSeconds_ -= deltaTime;
    if (remainingSeconds_ <= 0.0f)
        stateMachine_.ChangeState(BossStateType::Idle);
}

void BossDownState::Exit()
{
    if (boss_.GetHealth() > 0)
        boss_.ResetDownGauge();
}