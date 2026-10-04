#pragma once
#pragma once

#include "BossBaseState.h"
#include "AttackStateMachine.h"

class BossAttackState : public BossBaseState
{
public:
	BossAttackState(BossStateMachine& stateMachine, Boss& boss);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;
private:
	AttackStateMachine attackStateMachine_;
};
