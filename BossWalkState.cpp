#include "BossWalkState.h"
#include "Boss.h"

#include "DxPlus/Debug.h"

BossWalkState::BossWalkState(BossStateMachine& stateMachine, Boss& boss)
    : BossBaseState(stateMachine, boss)
{
}

void BossWalkState::Enter()
{
	walkMotion_.Reset();
	walkMotion_.SetDuration(3.0f); // 3秒で歩き状態を完了する
}

void BossWalkState::Tick(float deltaTime)
{
	walkMotion_.Update(deltaTime);
	if (walkMotion_.IsFinished())
	{
		walkMotion_.Reset();
		stateMachine_.ChangeState(BossStateType::Attack);
	}
	else
	{
		// 進行率に応じて移動方向を設定する
		float progress = walkMotion_.GetProgress();
		Vec3 moveDirection = Vec3(1.0f, 0.0f, 0.0f); // 右方向に移動する例
		float speed = 200.0f; // 移動速度
		boss_.SetMoveDirection(moveDirection, speed);
	}
	DxPlus::Debug::SetString(L"CurrentBossState:Walk");
}

void BossWalkState::Exit()
{
    boss_.StopMove();
}
