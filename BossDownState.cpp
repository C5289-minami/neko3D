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
    // 全体速度を上げた分、ダウン状態の維持時間を短くする。
    remainingSeconds_ =
        Const::BOSS_FLINCH_HOLD_SECONDS / SequenceRate;

    reachedLoopPoint_ = false;

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
    // speedScaleを倍率分だけ速くする。
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

    // Flinchが5フレーム目に到達したら、
    // Downへのブレンドを開始する。
    if (!reachedLoopPoint_ &&
        boss_.GetAnimation().IsFinished())
    {
        reachedLoopPoint_ = true;

        boss_.SetAnimation(
            BossAnimType::Down,
            true,
            true,

            // 速度倍率が大きいほどブレンド時間を短くする。
            BaseDownBlendSeconds / SequenceRate,

            Motion::Easing::EaseIn
        );

        // ブレンド完了後に再生されるDownアニメーションの速度。
        boss_.GetAnimation().SetSpeedScale(
            Const::ANIM_FPS
        );

        return;
    }

    // Flinch再生中とDownへのブレンド中は、
    // ダウン維持時間を減らさない。
    if (!reachedLoopPoint_ ||
        boss_.GetAnimation().IsBlending())
    {
        return;
    }

    // Downのループ再生開始後に時間を計測する。
    remainingSeconds_ -= deltaTime;

    if (remainingSeconds_ <= 0.0f)
    {
        stateMachine_.ChangeState(BossStateType::Idle);
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