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
    player_.SetAnimation(PlayerAnimType::‚¯‚Ã‚­‚ë‚¢, true);

    DxPlus::Debug::SetFormatString(L"PlayerKedukuroiState::Enter");
}

void PlayerKedukuroiState::Tick(float)
{
    const PlayerInput& input = player_.GetInput();
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
