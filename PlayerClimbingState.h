#pragma once
#include "PlayerBaseState.h"

// 壁登り用のステート。遷移条件と移動処理は後で接続する。
class PlayerClimbingState : public PlayerBaseState
{
public:
    PlayerClimbingState(PlayerStateMachine& stateMachine, Player& player);

    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;
};
