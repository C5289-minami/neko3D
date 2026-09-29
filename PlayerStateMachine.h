#pragma once
#include "StateMachine.h"
#include "PlayerStateType.h"

// State
#include "PlayerIdleState.h"
#include "PlayerWalkState.h"

class Player;

class PlayerStateMachine : public StateMachine
{
public:
	explicit PlayerStateMachine(Player& player);

	void Initialize();

	void ChangeState(PlayerStateType type);

	PlayerStateType GetCurrentType() const { return currentType_; };

private:
	State* FindState(PlayerStateType type);

	Player& player_;
	
	PlayerStateType currentType_{ PlayerStateType::None };

	PlayerIdleState idleState_;
	PlayerWalkState walkState_;
	// ‘¼‚ð‚±‚±‚É’Ç‰Á
};
