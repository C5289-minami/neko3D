#pragma once
#include "BossBaseState.h"
#include "Motion.h"

class BossAttackState : public BossBaseState
{
public:
	BossAttackState(BossStateMachine& stateMachine, Boss& boss);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;
private:
	Motion attackMotion_;
};
