#include "PlayerWallJump.h"
#include "DxPlus/Debug.h"
#include "Player.h"

PlayerWallJump::PlayerWallJump(PlayerStateMachine& stateMachine, Player& player)
	: PlayerBaseState(stateMachine, player)
{

}

void PlayerWallJump::Enter()
{
	player_.StopMove();
    player_.SetAnimation(PlayerAnimType::Double_Jump, false);
    player_.StartWallJump();
	DxPlus::Debug::SetFormatString(L"PlayerWallJump::Enter");

}

void PlayerWallJump::Tick(float)
{
    if (player_.IsGrounded() || !player_.IsGravityEnabled())
    {
        stateMachine_.ChangeState(player_.GetInput().isMoving
            ? PlayerStateType::Walk : PlayerStateType::Idle);
        return;
    }
    DxPlus::Debug::SetFormatString(L"state:wall_jump");
}

void PlayerWallJump::Exit()
{
    player_.StopMove();
    DxPlus::Debug::SetFormatString(L"PlayerWallJump::Exit");
}
