#pragma once
#include "PlayerBaseState.h"

class PlayerKedukuroiState : public PlayerBaseState
{
public:
	PlayerKedukuroiState(PlayerStateMachine& stateMachine, Player& player);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;
};
