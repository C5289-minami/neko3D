#pragma once
#include "BossBaseState.h"

class BossWalkState : public BossBaseState
{
public:
    BossWalkState(BossStateMachine& stateMachine, Boss& boss);

    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;
};
