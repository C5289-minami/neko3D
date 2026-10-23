#pragma once
#include "PlayerBaseState.h"
#include "Vector3.h"

// 壁を押し続けると登る。移動入力を離すと、通常の重力で落下する。
class PlayerClimbingState : public PlayerBaseState
{
public:
    PlayerClimbingState(PlayerStateMachine& stateMachine, Player& player);

    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;

private:
    float pushTime_{ 0.0f };
    Vec3 pushDirection_{};
};
