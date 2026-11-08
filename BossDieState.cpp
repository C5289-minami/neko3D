#include "BossDieState.h"
#include "Boss.h"
#include "Consts.h"

#include "DxPlus/Debug.h"

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

	startPosition_ = boss_.GetPosition();
	startAlpha_ = boss_.GetModelObject().GetAlpha();

	dieMotion_.Start(
		0.0f,
		1.0f,
		Const::BossDieFadeOutDuration,
		Motion::Easing::EaseIn
	);
}

void BossDieState::Tick(float deltaTime)
{
    DxPlus::Debug::SetString(L"CurrentBossState:Die");

    boss_.UpdateAnimation(deltaTime);
    dieMotion_.Update(deltaTime);

    const float progress = dieMotion_.GetEasedProgress();

    // 上昇
    boss_.SetPosition(startPosition_ + Vec3{
        0.0f,
        Const::BossDieUpLen * progress,
        0.0f
        });

    // フェードアウト
    boss_.GetModelObject().SetAlpha(startAlpha_ * (1.0f - progress));

    if (dieMotion_.IsFinished())
    {
        boss_.SetActive(false);
    }
}

void BossDieState::Exit()
{
	boss_.StartRotation();
}
