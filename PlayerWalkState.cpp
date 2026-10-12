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
    // WASD+Space　入力方向へ突進ジャンプ
    if (move.jumpPressed && move.isMoving && player_.IsGrounded() && player_.IsGravityEnabled())
    {
        stateMachine_.ChangeState(PlayerStateType::LungingJump);
        return;
    }
    // Spaceだけで真上にジャンプ
    if (move.jumpPressed && !move.isMoving && player_.IsGrounded() && player_.IsGravityEnabled())
    {
        stateMachine_.ChangeState(PlayerStateType::NeutralJump);
        return;
    }
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
