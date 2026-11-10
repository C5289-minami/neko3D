#include "BossFlinchState.h"
#include "Boss.h"
#include "DxPlus/Debug.h"
#include "SoundManager.h"
#include "ResourceManager.h"

namespace
{
    constexpr float FlinchStartFrame = 3.0f;
}

BossFlinchState::BossFlinchState(BossStateMachine& stateMachine, Boss& boss)
	: BossBaseState(stateMachine, boss)
{
}

void BossFlinchState::Enter()
{
    boss_.StopRotation();
    boss_.StopMove();
    boss_.SetAnimation(BossAnimType::Flinch, false,true,0.07f);
    boss_.GetAnimation().SetTime(FlinchStartFrame);
    SoundManager::PlaySE(
        RM().GetSound(
            ResourceKeys::Sound_BossDump
        )
    );
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
