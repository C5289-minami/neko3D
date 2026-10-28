#include "PlayerDoubleJumpState.h"
#include "DxPlus/Debug.h"
#include "Player.h"

PlayerDoubleJumpState::PlayerDoubleJumpState(PlayerStateMachine& stateMachine, Player& player)
    : PlayerBaseState(stateMachine, player)
{
}

void PlayerDoubleJumpState::Enter()
{
    player_.StopMove();
    player_.SetAnimation(PlayerAnimType::Double_Jump, false);
    player_.StartDoubleJump();
    DxPlus::Debug::SetFormatString(L"PlayerDoubleJumpState::Enter");
}

void PlayerDoubleJumpState::Tick(float)
{
    if (player_.IsGrounded() || !player_.IsGravityEnabled())
    {
        if (player_.GetInput().isMoving)
            stateMachine_.ChangeState(PlayerStateType::Walk);
        else
            stateMachine_.ChangeState(PlayerStateType::Idle);
        return;
    }

    // 発動時の向きと3倍の水平速度はPlayer側で維持する。
    // Spaceを連打しても、着地するまで追加のジャンプはしない。
    DxPlus::Debug::SetFormatString(L"state:double_jump");
}

void PlayerDoubleJumpState::Exit()
{
    player_.StopMove();
    DxPlus::Debug::SetFormatString(L"PlayerDoubleJumpState::Exit");
}