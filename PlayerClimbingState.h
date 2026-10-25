#pragma once
#include "PlayerBaseState.h"
#include "Vector3.h"

// 柱の中心から自動上昇し、上端で小さく跳ねて通常の移動へ戻る。
class PlayerClimbingState : public PlayerBaseState
{
public:
    PlayerClimbingState(PlayerStateMachine& stateMachine, Player& player);

    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;

private:
    // float pushTime_{ 0.0f };
    // Vec3 pushDirection_{};
};
