#include "PlayerDieState.h"
#include "Player.h"
#include "DxPlus/Debug.h"

PlayerDieState::PlayerDieState(PlayerStateMachine& stateMachine, Player& player)
    : PlayerBaseState(stateMachine, player)
{
}

void PlayerDieState::Enter()
{
    player_.StopMove();
    player_.SetAnimation(PlayerAnimType::Die, false);
}

void PlayerDieState::Tick(float)
{
    player_.StopMove();
    DxPlus::Debug::SetFormatString(L"state:die");
}

void PlayerDieState::Exit()
{
    player_.StopMove();
}
