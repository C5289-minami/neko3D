#include "PlayerWalkState.h"
#include "DxPlus/Debug.h"
#include "Player.h"
#include "PlayerInput.h"
#include "Consts.h"

PlayerWalkState::PlayerWalkState(PlayerStateMachine& stateMachine, Player& player)
	: PlayerBaseState(stateMachine, player)
{
}

void PlayerWalkState::Enter()
{
}

void PlayerWalkState::Tick(float deltaTime)
{
	PlayerInput move = player_.GetInput();
	if (!move.isMoving)
	{
		stateMachine_.ChangeState(PlayerStateType::Idle);
		return;
	}

	player_.SetMoveDirection(move.moveDir, Const::PLAYER_MOVE_SPEED);


	DxPlus::Debug::SetString(L"PlayerWalkState::Tick");
}

void PlayerWalkState::Exit()
{
	player_.StopMove();
}
