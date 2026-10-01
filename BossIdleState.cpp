#include "BossIdleState.h"
#include "Boss.h"

#include "DxPlus/Debug.h"

BossIdleState::BossIdleState(BossStateMachine& stateMachine, Boss& boss)
    : BossBaseState(stateMachine, boss)
{
}

void BossIdleState::Enter()
{
    boss_.StopMove();
	idleMotion_.Reset();
	idleMotion_.SetDuration(1.0f); // 1秒で1サイクルのモーション
}

void BossIdleState::Tick(float deltaTime)
{
	idleMotion_.Update(deltaTime);
	if(idleMotion_.IsFinished())
	{
		idleMotion_.Reset(); // モーションをループさせる
		stateMachine_.ChangeState(BossStateType::Walk); // 1サイクル終了後に歩き状態に遷移
	}
	DxPlus::Debug::SetString(L"CurrentBossState:Idle");
}

void BossIdleState::Exit()
{
}
