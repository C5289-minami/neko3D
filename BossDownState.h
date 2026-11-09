#pragma once
#include "BossBaseState.h"

class BossDownState : public BossBaseState
{
public:
    BossDownState(BossStateMachine& stateMachine, Boss& boss);

    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;

private:
    float remainingSeconds_{};
    bool reachedLoopPoint_{ false };
    bool downLoopStarted_{ false };
    bool recoveryStarted_ = false;
};
