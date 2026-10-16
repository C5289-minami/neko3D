#include "PlayerNeutralJumpState.h"
#include "DxPlus/Debug.h"
#include "Player.h"

PlayerNeutralJumpState::PlayerNeutralJumpState(PlayerStateMachine& stateMachine, Player& player)
    : PlayerBaseState(stateMachine, player)
{
}

void PlayerNeutralJumpState::Enter()
{
    player_.StopMove();
    player_.SetAnimation(PlayerAnimType::Neutral_Jump, false);
    player_.JumpAction();
    DxPlus::Debug::SetFormatString(L"PlayerNeutralJumpState::Enter");
}

void PlayerNeutralJumpState::Tick(float)
{
    if (player_.IsGrounded() || !player_.IsGravityEnabled())
    {
        stateMachine_.ChangeState(player_.GetInput().isMoving
            ? PlayerStateType::Walk : PlayerStateType::Idle);
        return;
    }

    // その場ジャンプの後も、空中で移動方向を変えられる。
    player_.SetMoveDirection(player_.GetInput().moveDir);
    DxPlus::Debug::SetFormatString(L"state:neutral_jump");
}

void PlayerNeutralJumpState::Exit()
{
    player_.StopMove();
    DxPlus::Debug::SetFormatString(L"PlayerNeutralJumpState::Exit");
}
