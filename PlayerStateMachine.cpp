#include "PlayerStateMachine.h"

PlayerStateMachine::PlayerStateMachine(Player& player)
    : player_(player),
	currentType_(PlayerStateType::None),
	idleState_(*this, player),
	walkState_(*this, player)
{

}
void PlayerStateMachine::Initialize()
{
	currentType_ = PlayerStateType::None;

	ChangeState(PlayerStateType::Idle);
}

void PlayerStateMachine::ChangeState(PlayerStateType type)
{
	if (currentType_ == type) return;

	State* newState = FindState(type);
	if (!newState)return;

	currentType_ = type;
	StateMachine::SwitchState(newState);
}


State* PlayerStateMachine::FindState(PlayerStateType type)
{
	switch (type)
	{
	case PlayerStateType::Idle:
		return &idleState_;
		break;
	case PlayerStateType::Walk:
		return &walkState_;
		break;
	case PlayerStateType::Run:
		break;
	case PlayerStateType::Jump:
		break;
	case PlayerStateType::Landing:
		break;
	default:
		return nullptr;
		break;
	}
}
