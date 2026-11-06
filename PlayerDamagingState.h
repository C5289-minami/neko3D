#pragma once
#include "PlayerBaseState.h"
#include "Consts.h"

class PlayerDamagingState final : public PlayerBaseState
{
public:
    static constexpr float AvoidTime = Const::PLAYER_AVOID_WINDOW_SECONDS;

    PlayerDamagingState(PlayerStateMachine& stateMachine, Player& player);

    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;
    float GetAvoidTime() const { return avoidTimer_; }

private:
    float remainingTime_{};
    float avoidTimer_{};
};
