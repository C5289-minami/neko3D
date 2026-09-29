#pragma once
#include "PlayerBaseState.h"

class PlayerIdleState : public PlayerBaseState
{
public:
	PlayerIdleState(
		PlayerStateMachine& stateMachine,
		Player& player
	);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;
};