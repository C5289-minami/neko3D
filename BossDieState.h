#pragma once
#include "BossBaseState.h"
#include "Motion.h"

class BossDieState : public BossBaseState
{
public:
    BossDieState(BossStateMachine& stateMachine, Boss& boss);

    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;
private:
};
