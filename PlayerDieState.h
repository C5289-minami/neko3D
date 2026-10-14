#pragma once
#include "PlayerBaseState.h"

class PlayerDieState final : public PlayerBaseState
{
public:
    PlayerDieState(PlayerStateMachine& stateMachine, Player& player);

    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;
};
