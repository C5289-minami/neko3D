#pragma once
#include "State.h"

class Player;
class PlayerStateMachine;

class PlayerBaseState : public State
{
public:
	explicit PlayerBaseState(
		PlayerStateMachine& stateMachine,
		Player& player
	):	stateMachine_(stateMachine),
		player_(player)
	{
	}
protected:
	PlayerStateMachine& stateMachine_;
	Player& player_;
};