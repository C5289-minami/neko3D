#pragma once
#include "PlayerBaseState.h"

class PlayerGroomingState : public PlayerBaseState
{
public:
	PlayerGroomingState(PlayerStateMachine& stateMachine, Player& player);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;
};
