#include "BossDieState.h"
#include "Boss.h"
#include "Consts.h"

#include "DxPlus/Debug.h"

BossDieState::BossDieState(BossStateMachine& stateMachine, Boss& boss)
	: BossBaseState(stateMachine, boss)
{
}

void BossDieState::Enter()
{
	boss_.SetAnimation(BossAnimType::Die, false);
}

void BossDieState::Tick(float deltaTime)
{
	DxPlus::Debug::SetString(L"CurrentBossState:Die");
	boss_.UpdateAnimation(deltaTime);

	if (boss_.GetAnimation().IsFinished())
	{
		// Boss‚Ì€–Sˆ—‚ğs‚¤
	}
}

void BossDieState::Exit()
{

}
