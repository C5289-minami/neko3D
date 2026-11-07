#include "BossDownState.h"
#include "Boss.h"
#include "Consts.h"

#include "DxPlus/Debug.h"

BossDownState::BossDownState(BossStateMachine& stateMachine, Boss& boss)
    : BossBaseState(stateMachine, boss)
{
}

void BossDownState::Enter()
{
    remainingSeconds_ = Const::BOSS_FLINCH_HOLD_SECONDS;
    reachedLoopPoint_ = false;
    boss_.StopRotation();
    boss_.StopMove();
    boss_.SetAnimation(BossAnimType::Down, false);
}

void BossDownState::Tick(float deltaTime)
{
    DxPlus::Debug::SetString(L"CurrentBossState:Down");

    boss_.UpdateAnimation(deltaTime);

    // 最初の再生だけは0フレームから最後まで流し、以降は同じ区間をループする。
    if (!reachedLoopPoint_ && boss_.GetAnimation().IsFinished())
    {
        reachedLoopPoint_ = true;
        boss_.SetAnimation(BossAnimType::Flinch, true, false, 0.0f);
    }

    remainingSeconds_ -= deltaTime;
    if (remainingSeconds_ <= 0.0f)
        stateMachine_.ChangeState(BossStateType::Idle);
}

void BossDownState::Exit()
{
    boss_.StartRotation();
    if (boss_.GetHealth() > 0)
        boss_.ResetDownGauge();
}
