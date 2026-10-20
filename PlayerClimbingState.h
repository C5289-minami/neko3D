#pragma once
#include "PlayerBaseState.h"
#include "Vector3.h"

// 壁を押し続けると登り、入力をやめるとゆっくり滑り落ちる。
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
