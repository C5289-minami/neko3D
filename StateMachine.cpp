#include "StateMachine.h"

void StateMachine::SwitchState(State* newState)
{
	if (currentState_) {
		currentState_->Exit();
	}

	currentState_ = newState;

	if (!currentState_) return;
	currentState_->Enter();

}

void StateMachine::Tick(float deltaTime)
{
	if (currentState_) {
		currentState_->Tick(deltaTime);
	}
}
