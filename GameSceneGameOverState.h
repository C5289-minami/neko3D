#pragma once
#include "GameBaseState.h"

class GameSceneGameOverState : public GameBaseState
{
public:
	GameSceneGameOverState(GameStateMachine& gameStateMachine, GameScene& gameScene);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;
private:
	int selectedIndex_{ 0 };
};
