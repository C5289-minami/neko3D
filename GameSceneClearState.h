#pragma once
#include "GameBaseState.h"

class GameSceneClearState : public GameBaseState
{
public:
	GameSceneClearState(GameStateMachine& gameStateMachine, GameScene& gameScene);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;
private:
	int selectedIndex_{ 0 };
	bool handPrepared_{ false };
	bool handAnimationStarted_{ false };
};
