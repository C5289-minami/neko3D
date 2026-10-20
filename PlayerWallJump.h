#pragma once
#include "PlayerBaseState.h"

class PlayerWallJump : public PlayerBaseState
{
public:
    PlayerWallJump(PlayerStateMachine& stateMachine, Player& player);
    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;
};


