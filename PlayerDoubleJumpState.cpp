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
    stateMachine_.NotifyEvent(PlayerActionEvent::DoubleJumped);
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

    // 空中でも方向入力を読み取り、Player側で横速度に反映する。
    player_.SetMoveDirection(player_.GetInput().moveDir);

    // Spaceを連打しても、着地するまで追加のジャンプはしない。
    DxPlus::Debug::SetFormatString(L"state:double_jump");
}

void PlayerDoubleJumpState::Exit()
{
    player_.StopMove();
    DxPlus::Debug::SetFormatString(L"PlayerDoubleJumpState::Exit");
}