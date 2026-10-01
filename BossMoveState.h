#pragma once
#include "BossBaseState.h"

class BossMoveState : public BossBaseState
{
public:
    BossMoveState(BossStateMachine& stateMachine, Boss& boss);

    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;
};
