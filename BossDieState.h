#pragma once
#include "BossBaseState.h"
#include "Motion.h"
#include "Vector3.h"

class BossDieState : public BossBaseState
{
public:
    BossDieState(BossStateMachine& stateMachine, Boss& boss);

    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;
private:
    Motion dieMotion_;
    Vec3 startPosition_;
    float startAlpha_{};
};
