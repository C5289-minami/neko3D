#pragma once
#include "BossBaseState.h"
#include "Motion.h"

class BossIdleState : public BossBaseState
{
public:
    BossIdleState(BossStateMachine& stateMachine, Boss& boss);

    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;
private:
	Motion idleMotion_;
};
