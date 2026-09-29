#pragma once
#include "PlayerBaseState.h"


class PlayerWalkState :
    public PlayerBaseState
{
    public:
    PlayerWalkState(
        PlayerStateMachine& stateMachine,
        Player& player
    );
    void Enter() override;
    void Tick(float deltaTime) override;
	void Exit() override;

};

