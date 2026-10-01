#pragma once
#include "BossBaseState.h"
#include "Motion.h"

class BossRecoverState : public BossBaseState
{
public:
	BossRecoverState(BossStateMachine& stateMachine, Boss& boss);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;
private:
	Motion recoverMotion_;
};
