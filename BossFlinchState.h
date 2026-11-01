#pragma once
#include "BossBaseState.h"
#include "Motion.h"

class BossFlinchState : public BossBaseState
{
public:
    BossFlinchState(BossStateMachine& stateMachine, Boss& boss);

    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;
private:
};
