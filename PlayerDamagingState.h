#pragma once
#include "PlayerBaseState.h"

class PlayerDamagingState final : public PlayerBaseState
{
public:
    PlayerDamagingState(PlayerStateMachine& stateMachine, Player& player);

    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;

private:
    float remainingTime_{};
};
