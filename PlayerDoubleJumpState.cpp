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

    // 突進せず、現在の方向キーで通常速度の空中移動を行う。
    // Spaceを連打しても、着地するまで追加のジャンプはしない。
    player_.SetMoveDirection(player_.GetInput().moveDir);
    DxPlus::Debug::SetFormatString(L"state:double_jump");
}

void PlayerDoubleJumpState::Exit()
{
    player_.StopMove();
    DxPlus::Debug::SetFormatString(L"PlayerDoubleJumpState::Exit");
}