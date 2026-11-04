#pragma once
#include "PlayerBaseState.h"

class PlayerDoubleJumpState : public PlayerBaseState
{
public:
    PlayerDoubleJumpState(PlayerStateMachine& stateMachine, Player& player);
    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;
};
