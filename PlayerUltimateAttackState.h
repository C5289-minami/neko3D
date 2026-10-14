#pragma once
#include "PlayerBaseState.h"

class PlayerUltimateAttackState : public PlayerBaseState
{
public:
    PlayerUltimateAttackState(PlayerStateMachine& stateMachine, Player& player);
    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;

private:
    float remainingTime_{ 0.0f };
};
