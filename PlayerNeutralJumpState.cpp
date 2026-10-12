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
    player_.SetAnimation(PlayerAnimType::neutral_Jump, false);
    player_.JumpAction();
    DxPlus::Debug::SetFormatString(L"PlayerNeutralJumpState::Enter");
}

void PlayerNeutralJumpState::Tick(float)
{
    // ’…’n‚·‚é‚Ü‚Å‰¡ˆÚ“®‚µ‚È‚¢
    player_.StopMove();
    if (player_.IsGrounded() || !player_.IsGravityEnabled())
    {
        stateMachine_.ChangeState(player_.GetInput().isMoving
            ? PlayerStateType::Walk : PlayerStateType::Idle);
        return;
    }
    DxPlus::Debug::SetFormatString(L"state:neutral_jump");
}

void PlayerNeutralJumpState::Exit()
{
    player_.StopMove();
    DxPlus::Debug::SetFormatString(L"PlayerNeutralJumpState::Exit");
}
