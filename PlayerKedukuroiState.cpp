#include "PlayerKedukuroiState.h"
#include "DxPlus/Debug.h"
#include "Player.h"
#include "PlayerInput.h"

PlayerKedukuroiState::PlayerKedukuroiState(PlayerStateMachine& stateMachine, Player& player)
    : PlayerBaseState(stateMachine, player)
{
}

void PlayerKedukuroiState::Enter()
{
    player_.StopMove();
    player_.SetAnimation(PlayerAnimType::ÇØÇ√Ç≠ÇÎÇ¢, true);

    DxPlus::Debug::SetFormatString(L"PlayerKedukuroiState::Enter");
}

void PlayerKedukuroiState::Tick(float)
{
    const PlayerInput& input = player_.GetInput();
    // SpaceÇæÇØÇ≈ê^è„Ç…ÉWÉÉÉìÉv
    if (input.jumpPressed && !input.isMoving && player_.IsGrounded() && player_.IsGravityEnabled())
    {
        stateMachine_.ChangeState(PlayerStateType::NeutralJump);
        return;
    }
    if (input.isMoving)
    {
        stateMachine_.ChangeState(PlayerStateType::Walk);
        return;
    }

    if (input.kedukuroiPressed)
    {
        stateMachine_.ChangeState(PlayerStateType::Idle);
        return;
    }

    DxPlus::Debug::SetFormatString(L"state:kedukuroi");
}

void PlayerKedukuroiState::Exit()
{
    player_.StopMove();

    DxPlus::Debug::SetFormatString(L"PlayerKedukuroiState::Exit");
}
