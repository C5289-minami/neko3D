#pragma once
#include "PlayerBaseState.h"

class PlayerNeutralJumpState : public PlayerBaseState
{
public:
    PlayerNeutralJumpState(PlayerStateMachine& stateMachine, Player& player);
    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;
};
