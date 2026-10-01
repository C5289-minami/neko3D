#include "BossAttackState.h"
#include "Boss.h"

#include "DxPlus/Debug.h"

BossAttackState::BossAttackState(BossStateMachine& stateMachine, Boss& boss)
    : BossBaseState(stateMachine, boss)
{
}

void BossAttackState::Enter()
{
	attackMotion_.Reset();
	attackMotion_.SetDuration(1.0f); // 1•b‚ÅUŒ‚ƒ‚[ƒVƒ‡ƒ“‚ğŠ®—¹‚·‚é
}

void BossAttackState::Tick(float deltaTime)
{
	attackMotion_.Update(deltaTime);
	if (attackMotion_.IsFinished())
	{
		attackMotion_.Reset();
		stateMachine_.ChangeState(BossStateType::Recover);
	}
	DxPlus::Debug::SetString(L"CurrentBossState:Attack");
}

void BossAttackState::Exit()
{
}
