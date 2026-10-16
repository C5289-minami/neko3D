#include "PlayerLungingJumpState.h"
#include "DxPlus/Debug.h"
#include "Player.h"

PlayerLungingJumpState::PlayerLungingJumpState(PlayerStateMachine& stateMachine, Player& player)
    : PlayerBaseState(stateMachine, player)
{
}

void PlayerLungingJumpState::Enter()
{
    player_.StopMove();
    player_.SetAnimation(PlayerAnimType::Lunging_Jump, false);
    player_.StartLungingJump(player_.GetInput().moveDir);
    DxPlus::Debug::SetFormatString(L"PlayerLungingJumpState::Enter");
}

void PlayerLungingJumpState::Tick(float)
{
    // ‹ó’†‚Å‚Í“¥‚İØ‚Á‚½•ûŒü‚Öi‚İ‘±‚¯‚é
    if (player_.IsGrounded() || !player_.IsGravityEnabled())
    {
        stateMachine_.ChangeState(player_.GetInput().isMoving
            ? PlayerStateType::Walk : PlayerStateType::Idle);
        return;
    }
    DxPlus::Debug::SetFormatString(L"state:lunging_jump");
}

void PlayerLungingJumpState::Exit()
{
    // ’…’n‚µ‚½‚ç“Ëi‚ğ~‚ß‚é
    player_.StopMove();
    DxPlus::Debug::SetFormatString(L"PlayerLungingJumpState::Exit");
}
