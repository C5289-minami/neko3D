#pragma once

#include "AttackStateType.h"
#include "State.h"

class AttackStateMachine;

class AttackBaseState : public State
{
 public:
	AttackBaseState(AttackStateMachine& stateMachine, AttackStateType type);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;

	private:
	AttackStateMachine& stateMachine_;
	AttackStateType type_;
};