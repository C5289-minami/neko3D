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
    // ゲージ100で右クリックすると必殺技
    if (move.ultimatePressed && player_.CanUseUltimate())
    {
        stateMachine_.ChangeState(PlayerStateType::UltimateAttack);
        return;
    }
    // 左クリックで通常攻撃
    if (move.attackPressed)
    {
        stateMachine_.ChangeState(PlayerStateType::Attack);
        return;
    }
    if (move.jumpPressed && player_.TryJump())
    {
        return;
    }
	if (move.groomingPressed)
	{
		stateMachine_.ChangeState(PlayerStateType::Grooming);
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
