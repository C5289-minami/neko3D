#include "BossDieState.h"
#include "Boss.h"
#include "Consts.h"
#include "SoundManager.h"
#include "ResourceManager.h"

#include "DxPlus/Debug.h"
#include <algorithm>

namespace Const
{
	constexpr float BossDieUpLen = 1000.0f;
	constexpr float BossDieFadeOutDuration = 8.0f;
}

BossDieState::BossDieState(BossStateMachine& stateMachine, Boss& boss)
	: BossBaseState(stateMachine, boss)
{
}

void BossDieState::Enter()
{
    boss_.SetAnimation(BossAnimType::Die, false);
    boss_.StopRotation();
    boss_.StopMove();

    startPosition_ = boss_.GetPosition();
    startAlpha_ = boss_.GetModelObject().GetAlpha();

    boss_.CameraShakeRequest(
        CameraShakeType::Vertical,
        0.35f,
        20.0f,
        60.0f
    );

    dieMotion_.Start(
        0.0f,
        1.0f,
        Const::BossDieFadeOutDuration,
        Motion::Easing::EaseIn
    );

    SoundManager::PlaySE(
        RM().GetSound(
            ResourceKeys::Sound_BossLastDamage
        )
    );
}

void BossDieState::Tick(float deltaTime)
{
    DxPlus::Debug::SetString(L"CurrentBossState:Die");

    boss_.UpdateAnimation(deltaTime);
    dieMotion_.Update(deltaTime);

    const float progress = dieMotion_.GetEasedProgress();

    // 全体進捗が20%に達するまでは上昇させない。
    constexpr float RiseStartProgress = 0.2f;

    const float riseProgress = (std::clamp)(
        (progress - RiseStartProgress) /
        (1.0f - RiseStartProgress),
        0.0f,
        1.0f
        );

    // 上昇開始直後をさらにゆっくりにする。
    const float easedRiseProgress =
        riseProgress * riseProgress;

    // 撃破直後に少しだけ沈ませる。
    constexpr float SinkLength = 30.0f;

    const float sinkProgress = (std::clamp)(
        progress / RiseStartProgress,
        0.0f,
        1.0f
        );

    // 上昇開始後は沈み込みを徐々に戻す。
    const float sinkOffset =
        SinkLength *
        sinkProgress *
        (1.0f - riseProgress);

    const float riseOffset =
        Const::BossDieUpLen * easedRiseProgress;

    boss_.SetPosition(
        startPosition_ +
        Vec3{
            0.0f,
            riseOffset - sinkOffset,
            0.0f
        }
    );

    // 全体進捗が40%に達するまでは透明にしない。
    constexpr float FadeStartProgress = 0.4f;

    const float fadeProgress = (std::clamp)(
        (progress - FadeStartProgress) /
        (1.0f - FadeStartProgress),
        0.0f,
        1.0f
        );

    boss_.GetModelObject().SetAlpha(
        startAlpha_ * (1.0f - fadeProgress)
    );

    if (dieMotion_.IsFinished())
    {
        boss_.SetActive(false);
    }
}
void BossDieState::Exit()
{
	boss_.StartRotation();
}
