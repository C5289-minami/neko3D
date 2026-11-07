#include "BossFlinchState.h"
#include "Boss.h"
#include "DxPlus/Debug.h"

BossFlinchState::BossFlinchState(BossStateMachine& stateMachine, Boss& boss)
	: BossBaseState(stateMachine, boss)
{
}

void BossFlinchState::Enter()
{
    boss_.StopRotation();
    boss_.StopMove();
    boss_.SetAnimation(BossAnimType::Flinch, false);
}

void BossFlinchState::Tick(float deltaTime)
{
    DxPlus::Debug::SetString(L"CurrentBossState:Flinch");
    boss_.UpdateAnimation(deltaTime);
    if (boss_.GetAnimation().IsFinished())
        stateMachine_.ChangeState(BossStateType::Idle);
}

void BossFlinchState::Exit()
{
	boss_.StartRotation();
    boss_.SetAnimation(BossAnimType::Idle, true);
}
