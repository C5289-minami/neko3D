#include "PlayerIdleState.h"
#include "DxPlus/DxPlus.h"
#include "Player.h"
#include "PlayerInput.h"
#include "Consts.h"

PlayerIdleState::PlayerIdleState(PlayerStateMachine& stateMachine, Player& player)
	: PlayerBaseState(stateMachine, player)
{

}

void PlayerIdleState::Enter()
{
	player_.StopMove();
	player_.SetAnimation(PlayerAnimType::Idle, true);

	DxPlus::Debug::SetFormatString(L"PlayerIdleState::Enter");
}

void PlayerIdleState::Tick(float)
{
	const PlayerInput& move = player_.GetInput();
    // SpaceÇæÇØÇ≈ê^è„Ç…ÉWÉÉÉìÉv
    if (move.jumpPressed &&!move.isMoving &&
		player_.IsGrounded() &&player_.IsGravityEnabled())
    {
        stateMachine_.ChangeState(PlayerStateType::NeutralJump);
        return;
    }
	if (move.kedukuroiPressed)
	{
		stateMachine_.ChangeState(PlayerStateType::Kedukuroi);
		return;
	}
	if(move.isMoving)
	{
		stateMachine_.ChangeState(PlayerStateType::Walk);
		return;
	}


	DxPlus::Debug::SetFormatString(L"state:idle");
}

void PlayerIdleState::Exit()
{
	DxPlus::Debug::SetFormatString(L"PlayerIdleState::Exit");
}
