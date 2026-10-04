#include "BossAttackState.h"
#include "Boss.h"

#include "DxPlus/Debug.h"

BossAttackState::BossAttackState(BossStateMachine& stateMachine, Boss& boss)
    : BossBaseState(stateMachine, boss)
{
}

void BossAttackState::Enter()
{
	attackStateMachine_.StartAttack(boss_.GetActiveSkill());
}

void BossAttackState::Tick(float deltaTime)
{
    attackStateMachine_.Tick(deltaTime);
	if (attackStateMachine_.IsFinished())
	{
		boss_.ClearActiveSkill();
		stateMachine_.ChangeState(BossStateType::Idle);
	}
	DxPlus::Debug::SetString(L"CurrentBossState:Attack");
}

void BossAttackState::Exit()
{
	attackStateMachine_.Initialize();
	boss_.ClearActiveSkill();
    boss_.SetAnimation(BossAnimType::Idle, true); // 攻撃終了後にアニメーションをIdleに戻す
}
