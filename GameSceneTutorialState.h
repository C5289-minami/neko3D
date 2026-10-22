#pragma once
#include "GameBaseState.h"

class GameSceneTutorialState : public GameBaseState
{
public:
	GameSceneTutorialState(GameStateMachine& gameStateMachine, GameScene& gameScene);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;
};