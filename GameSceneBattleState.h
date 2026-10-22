#pragma once
#include "GameBaseState.h"

class GameSceneBattleState : public GameBaseState
{
public:
	GameSceneBattleState(GameStateMachine& gameStateMachine, GameScene& gameScene);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;
};

