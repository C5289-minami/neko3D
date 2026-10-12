#include "PlayerWalkState.h"
#include "DxPlus/Debug.h"
#include "Player.h"
#include "PlayerInput.h"

PlayerWalkState::PlayerWalkState(PlayerStateMachine& stateMachine, Player& player)
	: PlayerBaseState(stateMachine, player)
{
}

void PlayerWalkState::Enter()
{
   player_.SetAnimation(PlayerAnimType::Walk, true);
}

void PlayerWalkState::Tick(float)
{
	const PlayerInput& move = player_.GetInput();
	if (move.kedukuroiPressed)
	{
		stateMachine_.ChangeState(PlayerStateType::Kedukuroi);
		return;
	}
	if (!move.isMoving)
	{
		stateMachine_.ChangeState(PlayerStateType::Idle);
		return;
	}

	player_.SetMoveDirection(move.moveDir);


	DxPlus::Debug::SetFormatString(L"state:walk");
}

void PlayerWalkState::Exit()
{
	player_.StopMove();
}
