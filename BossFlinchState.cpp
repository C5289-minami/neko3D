#include "BossFlinchState.h"
#include "Boss.h"
#include "Consts.h"

#include "DxPlus/Debug.h"

BossFlinchState::BossFlinchState(BossStateMachine& stateMachine, Boss& boss)
	: BossBaseState(stateMachine, boss)
{
}

void BossFlinchState::Enter()
{
	boss_.SetAnimation(BossAnimType::Damage, false);
}

void BossFlinchState::Tick(float deltaTime)
{
	DxPlus::Debug::SetString(L"CurrentBossState:Flinch");
	boss_.UpdateAnimation(deltaTime);

	if (boss_.GetAnimation().IsFinished())
	{
		// Bossのフリント処理が終了したら、次の状態に遷移する
		stateMachine_.ChangeState(BossStateType::Idle);
	}

}

void BossFlinchState::Exit()
{
}
