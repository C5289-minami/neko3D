#pragma once
#pragma once
#include "StateMachine.h"
#include "GameStateType.h"

// State
#include "GameSceneTitleState.h"
#include "GameSceneSelectState.h"
#include "GameSceneTutorialState.h"
#include "GameSceneBattleState.h"
#include "GameSceneClearState.h"
#include "GameSceneGameOverState.h"

class GameScene;

class GameStateMachine : public StateMachine
{
public:
	explicit GameStateMachine(GameScene& gameScene);

	void Initialize();

	void ChangeState(GameStateType type);
	void StartBattleWithoutIntro();

	GameStateType GetCurrentType() const { return currentType_; };

private:
	State* FindState(GameStateType type);

	GameScene& gameScene_;

	GameStateType currentType_{ GameStateType::Title };
	bool initialized_{ false };

	GameSceneTitleState titleState_;
	GameSceneSelectState selectState_;
	GameSceneTutorialState tutorialState_;
	GameSceneBattleState battleState_;
	GameSceneClearState clearState_;
	GameSceneGameOverState gameOverState_;
};
