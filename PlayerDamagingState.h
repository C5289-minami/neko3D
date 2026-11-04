#pragma once
#include "PlayerBaseState.h"

class PlayerDamagingState final : public PlayerBaseState
{
public:
    static constexpr float AvoidTime = 0.3f;

    PlayerDamagingState(PlayerStateMachine& stateMachine, Player& player);

    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;
    float GetAvoidTime() const { return avoidTimer_; }

private:
    float remainingTime_{};
    float avoidTimer_{};
};
