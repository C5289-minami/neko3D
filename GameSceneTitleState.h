#pragma once
#include "GameBaseState.h"

class GameSceneTitleState : public GameBaseState
{
public:
	GameSceneTitleState(GameStateMachine& gameStateMachine, GameScene& gameScene);
	
	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;
};
