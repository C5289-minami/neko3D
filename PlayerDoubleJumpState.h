#pragma once
#include "PlayerBaseState.h"

class PlayerDoubleJumpState : public PlayerBaseState
{
public:
    static constexpr float AvoidTime = 0.3f;

    PlayerDoubleJumpState(PlayerStateMachine& stateMachine, Player& player);
    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;
    bool TryAvoid();
private:
    float avoidTimer_{};
};
