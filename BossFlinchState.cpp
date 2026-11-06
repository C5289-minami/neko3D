#include "BossFlinchState.h"
#include "Boss.h"
#include "Consts.h"
#include "Collision.h"

#include "DxPlus/Debug.h"

BossFlinchState::BossFlinchState(BossStateMachine& stateMachine, Boss& boss)
	: BossBaseState(stateMachine, boss)
{
}

void BossFlinchState::Enter()
{
	boss_.StopRotation();
    remainingSeconds_ = FlinchHoldSeconds;
    boss_.StopMove();
    boss_.SetAnimation(BossAnimType::Flach, true);
}

void BossFlinchState::Tick(float deltaTime)
{
    boss_.UpdateAnimation(deltaTime);
    

    DxPlus::Debug::SetString(L"CurrentBossState:Flinch");
    // ダウンアニメーションをループ再生しながら5秒待つ。
    remainingSeconds_ -= deltaTime;
    if (remainingSeconds_ <= 0.0f)
        stateMachine_.ChangeState(BossStateType::Idle);
}

void BossFlinchState::Exit()
{
	boss_.StartRotation();
    if (boss_.GetHealth() > 0)
        boss_.ResetDownGauge();
}
