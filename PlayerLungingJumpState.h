#pragma once
#include "PlayerBaseState.h"

class PlayerLungingJumpState : public PlayerBaseState
{
public:
    PlayerLungingJumpState(PlayerStateMachine& stateMachine, Player& player);
    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;
};
