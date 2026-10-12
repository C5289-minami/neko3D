#include "PlayerStateMachine.h"

PlayerStateMachine::PlayerStateMachine(Player& player)
    : player_(player),
	currentType_(PlayerStateType::Idle),
	idleState_(*this, player),
	walkState_(*this, player),
	kedukuroiState_(*this, player),
	neutralJumpState_(*this, player)
{

}
void PlayerStateMachine::Initialize()
{
	initialized_ = false;

	ChangeState(PlayerStateType::Idle);
}

void PlayerStateMachine::ChangeState(PlayerStateType type)
{
	if (initialized_ && currentType_ == type) return;

	State* newState = FindState(type);
	if (!newState)return;

	currentType_ = type;
	initialized_ = true;
	StateMachine::SwitchState(newState);

}


State* PlayerStateMachine::FindState(PlayerStateType type)
{
    switch (type)
    {
    case PlayerStateType::Idle:
        return &idleState_;
    case PlayerStateType::Walk:
        return &walkState_;
    case PlayerStateType::Kedukuroi:
        return &kedukuroiState_;
    case PlayerStateType::NeutralJump:
        return &neutralJumpState_;
    default:
        return nullptr;
    }
}
