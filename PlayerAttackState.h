#pragma once
#include "PlayerBaseState.h"

class PlayerAttackState : public PlayerBaseState
{
public:
    PlayerAttackState(PlayerStateMachine& stateMachine, Player& player);
    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;
};
