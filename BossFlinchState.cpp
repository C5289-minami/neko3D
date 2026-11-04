#include "BossFlinchState.h"
#include "Boss.h"
#include "Consts.h"

#include "DxPlus/Debug.h"

BossFlinchState::BossFlinchState(BossStateMachine& stateMachine, Boss& boss)
	: BossBaseState(stateMachine, boss)
{
}

void BossFlinchState::Enter()
{
    remainingSeconds_ = FlinchDurationSeconds;
    boss_.StopMove();
    boss_.SetAnimation(BossAnimType::Flach, false);
}

void BossFlinchState::Tick(float deltaTime)
{
    DxPlus::Debug::SetString(L"CurrentBossState:Flinch");
    boss_.UpdateAnimation(deltaTime);
    remainingSeconds_ -= deltaTime;
    if (remainingSeconds_ <= 0.0f)
        stateMachine_.ChangeState(BossStateType::Idle);
}

void BossFlinchState::Exit()
{
    if (boss_.GetHealth() > 0)
        boss_.ResetDownGauge();
}
