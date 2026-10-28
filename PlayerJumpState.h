#pragma once
#include "PlayerBaseState.h"

class PlayerJumpState : public PlayerBaseState
{
public:
    PlayerJumpState(PlayerStateMachine& stateMachine, Player& player);
    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;
};
