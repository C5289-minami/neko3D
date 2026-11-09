#include "BossDownState.h"
#include "Boss.h"
#include "Consts.h"

#include "DxPlus/Debug.h"

namespace
{
    constexpr float FlinchEndFrame = 6.0f;

    // SequenceRateが1.0のときのブレンド時間
    constexpr float BaseDownBlendSeconds = 0.8f;

    // 2.0なら全体が2倍速、0.5なら全体が半分の速度
    constexpr float SequenceRate = 0.3f;
}

BossDownState::BossDownState(
    BossStateMachine& stateMachine,
    Boss& boss)
    : BossBaseState(stateMachine, boss)
{
}

void BossDownState::Enter()
{
    remainingSeconds_ =
        Const::BOSS_FLINCH_HOLD_SECONDS / SequenceRate;

    reachedLoopPoint_ = false;
    downLoopStarted_ = false;
    recoveryStarted_ = false;

    boss_.StopRotation();
    boss_.StopMove();

    // Flinchへ即時切り替え、自動再生はさせない。
    boss_.SetAnimation(
        BossAnimType::Flinch,
        false,
        false,
        0.0f
    );

    // Flinchを0～5フレームまで再生する。
    boss_.GetAnimation().SetTime(0.0f);
    boss_.GetAnimation().PlayTo(
        FlinchEndFrame,
        Const::ANIM_FPS * SequenceRate
    );
}


void BossDownState::Tick(float deltaTime)
{
    DxPlus::Debug::SetString(L"CurrentBossState:Down");

    boss_.UpdateAnimation(deltaTime);

    // Flinch終了後、Downへのブレンドを開始する。
    if (!reachedLoopPoint_ &&
        boss_.GetAnimation().IsFinished())
    {
        reachedLoopPoint_ = true;

        boss_.SetAnimation(
            BossAnimType::Down,
            true,
            true,
            BaseDownBlendSeconds / SequenceRate,
            Motion::Easing::EaseIn
        );

        boss_.GetAnimation().SetSpeedScale(Const::ANIM_FPS);
        return;
    }

    // Downへのブレンド完了を1回だけ検出する。
    if (reachedLoopPoint_ &&
        !downLoopStarted_ &&
        !boss_.GetAnimation().IsBlending())
    {
        downLoopStarted_ = true;

        // 画面を縦方向に揺らす。
        boss_.CameraShakeRequest(
            CameraShakeType::Vertical,
            0.4f,
            30.0f,
            80.0f
        );
    }

    // Flinch再生中とDownへのブレンド中は維持時間を減らさない。
    if (!downLoopStarted_)
    {
        return;
    }

    // Idleへの復帰ブレンドを開始済みなら、完了を待つ。
    if (recoveryStarted_)
    {
        if (!boss_.GetAnimation().IsBlending())
        {
            stateMachine_.ChangeState(BossStateType::Idle);
        }

        return;
    }

    remainingSeconds_ -= deltaTime;

    if (remainingSeconds_ <= 0.0f)
    {
        recoveryStarted_ = true;

        // Idleは再生せず、先頭フレームへゆっくりブレンドする。
        boss_.SetAnimation(
            BossAnimType::Idle,
            false,
            false,
            1.5f
        );
    }
}

void BossDownState::Exit()
{
    boss_.StartRotation();

    if (boss_.GetHealth() > 0)
    {
        boss_.ResetDownGauge();
    }
}