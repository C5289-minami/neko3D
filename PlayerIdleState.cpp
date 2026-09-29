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

	DxPlus::Debug::SetString(L"PlayerIdleState::Enter");
}

void PlayerIdleState::Tick(float deltaTime)
{
	PlayerInput move = player_.GetInput();
	if(move.isMoving)
	{
		stateMachine_.ChangeState(PlayerStateType::Walk);
		return;
	}


	DxPlus::Debug::SetString(L"PlayerIdleState::Tick");
}

void PlayerIdleState::Exit()
{
	DxPlus::Debug::SetString(L"PlayerIdleState::Exit");
}
