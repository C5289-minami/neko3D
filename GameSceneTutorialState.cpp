#include "GameSceneTutorialState.h"
#include "GameSceneTutorialState.h"
#include "GameScene.h"
#include "GameStateMachine.h"
#include "DxPlus/Debug.h"

GameSceneTutorialState::GameSceneTutorialState(GameStateMachine& gameStateMachine, GameScene& gameScene)
	: GameBaseState(gameStateMachine, gameScene)
{
}

void GameSceneTutorialState::Enter()
{
	DxPlus::Debug::SetFormatString(L"GameSceneBattleState::Enter");
}

void GameSceneTutorialState::Tick(float deltaTime)
{
	DxPlus::Debug::SetFormatString(L"GameSceneBattleState::Enter");
    const bool battleRequested = DxLib::CheckHitKey(KEY_INPUT_F2);
	if (battleRequested)
	{
		stateMachine_.ChangeState(GameStateType::Battle);
	}
	else
	{
		gameScene_.FocusTutorialCamera();
	}

	const std::string requestedShot = battleRequested ? "Battle" : "Tutorial";
	if (gameScene_.UpdateStateCamera(deltaTime, requestedShot))
	{
       if (battleRequested)
		{
			stateMachine_.ChangeState(GameStateType::Title);
		}
		else
		{
			gameScene_.OnTutorialCameraTransitionCompleted();
		}
	}
}

void GameSceneTutorialState::Exit()
{
	DxPlus::Debug::SetFormatString(L"GameSceneBattleState::Enter");
}
