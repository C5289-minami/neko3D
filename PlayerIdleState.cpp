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
    // 1回目のSpaceは、方向キーを押していても通常ジャンプにする。
    // 空中で押し直した2回目のSpaceは、ダブルジャンプに使う。
    if (move.jumpPressed && player_.IsGrounded() && player_.IsGravityEnabled())
    {
        stateMachine_.ChangeState(PlayerStateType::NeutralJump);
        return;
    }
	if (move.groomingPressed)
	{
		stateMachine_.ChangeState(PlayerStateType::Grooming);
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
