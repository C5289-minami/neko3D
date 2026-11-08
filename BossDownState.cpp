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
    downLoopStarted_ = false;

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

        // ここにループ開始時の処理を書く。

		// 画面を縦方向に揺らす。
        boss_.CameraShakeRequest(
            CameraShakeType::Vertical,
            0.4f,
            30.0f,
            80.0f
		);
    }

    // Flinch再生中とブレンド中は維持時間を減らさない。
    if (!downLoopStarted_)
    {
        return;
    }

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