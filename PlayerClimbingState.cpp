#include "PlayerClimbingState.h"
#include "DxPlus/Debug.h"
#include "Player.h"
#include <algorithm>

PlayerClimbingState::PlayerClimbingState(PlayerStateMachine& stateMachine, Player& player)
    : PlayerBaseState(stateMachine, player)
{
}

void PlayerClimbingState::Enter()
{
    player_.StopMove();
    player_.SetVerticalSpeed(0.0f);
    // pushTime_ = 0.0f;
    // pushDirection_ = {};

    player_.SetAnimation(PlayerAnimType::Walk, true);
   
    DxPlus::Debug::SetFormatString(L"PlayerClimbingState::Enter");
}


void PlayerClimbingState::Tick(float deltaTime)
{
    player_.UpdateAnimation(deltaTime * 2.0f);
	// 柱の自動上昇が終了したら、ジャンプ状態に遷移する。
    if (!player_.IsPillarElevatorActive() || !player_.IsGravityEnabled())
    {
        stateMachine_.ChangeState(PlayerStateType::Jump);
        return;
    }

	// 柱の上端に到達したら、ジャンプ状態に遷移する。
    if (player_.UpdatePillarElevator(deltaTime))
    {
        bool shouldExitJump = !player_.GetInput().isMoving;
        stateMachine_.ChangeState(PlayerStateType::Jump);
        if (shouldExitJump)
            player_.StartPillarExitJump();
        // 登らせる補正をかける
		Vec3 elevatorCenter = player_.GetPillarElevatorCenter();
        elevatorCenter.y = player_.GetPosition().y; // Y座標は現在の位置を維持
        player_.SetPosition(elevatorCenter);
        stateMachine_.NotifyEvent(PlayerActionEvent::ReachedPillarTop);
    }
    DxPlus::Debug::SetFormatString(L"PlayerClimbingState::Tick");
}

void PlayerClimbingState::Exit()
{
    player_.EndPillarElevator();
    player_.StopMove();
    player_.SetVerticalSpeed(0.0f);
    DxPlus::Debug::SetFormatString(L"PlayerClimbingState::Exit");
}
