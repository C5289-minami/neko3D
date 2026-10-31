#include "PlayerGroomingState.h"
#include "DxPlus/Debug.h"
#include "Player.h"
#include "PlayerInput.h"

PlayerGroomingState::PlayerGroomingState(PlayerStateMachine& stateMachine, Player& player)
    : PlayerBaseState(stateMachine, player)
{
}

void PlayerGroomingState::Enter()
{
    player_.StopMove();
    player_.SetAnimation(PlayerAnimType::Grooming, true);

    DxPlus::Debug::SetFormatString(L"PlayerGroomingState::Enter");
}

void PlayerGroomingState::Tick(float)
{
    const PlayerInput& input = player_.GetInput();
    // ゲージ100で右クリックすると必殺技
    if (input.ultimatePressed && player_.CanUseUltimate())
    {
        stateMachine_.ChangeState(PlayerStateType::UltimateAttack);
        return;
    }
    // 左クリックで通常攻撃
    if (input.attackPressed)
    {
        stateMachine_.ChangeState(PlayerStateType::Attack);
        return;
    }
    if (input.jumpPressed && player_.TryJump())
    {
        return;
    }
    if (input.isMoving)
    {
        stateMachine_.ChangeState(PlayerStateType::Walk);
        return;
    }

    if (input.groomingPressed)
    {
        stateMachine_.ChangeState(PlayerStateType::Idle);
        return;
    }

    DxPlus::Debug::SetFormatString(L"state:grooming");
}

void PlayerGroomingState::Exit()
{
    player_.StopMove();

    DxPlus::Debug::SetFormatString(L"PlayerGroomingState::Exit");
}
