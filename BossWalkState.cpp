#include "BossWalkState.h"
#include "Boss.h"
#include "Consts.h"

#include "DxPlus/Debug.h"

BossWalkState::BossWalkState(BossStateMachine& stateMachine, Boss& boss)
    : BossBaseState(stateMachine, boss)
{
}

void BossWalkState::Enter()
{
	walkMotion_.Reset();
  walkMotion_.SetDuration(Const::BOSS_WALK_MOTION_DURATION);
}

void BossWalkState::Tick(float deltaTime)
{
	walkMotion_.Update(deltaTime);
	if (walkMotion_.IsFinished())
	{
		walkMotion_.Reset();
		stateMachine_.ChangeState(BossStateType::Idle);
	}
	else
	{
		// is—¦‚É‰‚¶‚ÄˆÚ“®•ûŒü‚ğİ’è‚·‚é
		float progress = walkMotion_.GetProgress();
		Vec3 moveDirection = Vec3(1.0f - progress, 0.0f, progress - 1.0f); // ‰E•ûŒü‚ÉˆÚ“®‚·‚é—á
       boss_.SetMoveDirection(moveDirection, Const::BOSS_MOVE_SPEED);
	}
	DxPlus::Debug::SetString(L"CurrentBossState:Walk");
}

void BossWalkState::Exit()
{
    boss_.StopMove();
}
