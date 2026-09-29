#pragma once
#include "State.h"

class StateMachine
{
public:
	// ó‘Ô‚ğ•ÏX‚·‚é
	void SwitchState(State* newState);

	// Œ»İ‚Ìó‘Ô‚ğXV‚·‚é
	void Tick(float deltaTime);

private:
	State* currentState_{ nullptr };
};

