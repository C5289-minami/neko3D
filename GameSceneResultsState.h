#pragma once
#include "GameBaseState.h"

class GameSceneResultsState : public GameBaseState
{
public:
	GameSceneResultsState(GameStateMachine& gameStateMachine, GameScene& gameScene);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;
};
