#pragma once
#include "GameBaseState.h"

class GameSceneSelectState : public GameBaseState
{
public:
	GameSceneSelectState(GameStateMachine& gameStateMachine, GameScene& gameScene);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;
};
