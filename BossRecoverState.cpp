#include "BossRecoverState.h"
#include "Boss.h"

#include "DxPlus/Debug.h"

BossRecoverState::BossRecoverState(BossStateMachine& stateMachine, Boss& boss)
    : BossBaseState(stateMachine, boss)
{
}

void BossRecoverState::Enter()
{
	recoverMotion_.Reset();
	recoverMotion_.SetDuration(1.0f); // 1•b‚Å‰ñ•œƒ‚[ƒVƒ‡ƒ“‚ğŠ®—¹‚·‚é
}

void BossRecoverState::Tick(float deltaTime)
{
	recoverMotion_.Update(deltaTime);
	if (recoverMotion_.IsFinished())
	{
		recoverMotion_.Reset();
		stateMachine_.ChangeState(BossStateType::Idle);
	}
	DxPlus::Debug::SetString(L"CurrentBossState:Recover");
}

void BossRecoverState::Exit()
{
}
