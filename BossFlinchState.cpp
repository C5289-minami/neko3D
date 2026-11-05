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
    remainingSeconds_ = FlinchHoldSeconds;
    boss_.StopMove();
    boss_.SetAnimation(BossAnimType::Flach, false);
}

void BossFlinchState::Tick(float deltaTime)
{
    DxPlus::Debug::SetString(L"CurrentBossState:Flinch");
    // 最後まで再生する。終了したフレームの時間は、姿勢を保つ5秒に含めない。
    if (!boss_.GetAnimation().IsFinished())
    {
        boss_.UpdateAnimation(deltaTime);
        return;
    }
    // アニメーションの最後の姿勢を保ったまま、5秒待つ。
    remainingSeconds_ -= deltaTime;
    if (remainingSeconds_ <= 0.0f)
        stateMachine_.ChangeState(BossStateType::Idle);
}

void BossFlinchState::Exit()
{
    if (boss_.GetHealth() > 0)
        boss_.ResetDownGauge();
}
