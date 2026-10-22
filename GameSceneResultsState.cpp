#include "GameSceneResultsState.h"
#include "GameSceneResultsState.h"
#include "GameScene.h"
#include "GameStateMachine.h"
#include "DxPlus/Debug.h"

GameSceneResultsState::GameSceneResultsState(GameStateMachine& gameStateMachine, GameScene& gameScene)
	:GameBaseState(gameStateMachine, gameScene)
{
}

void GameSceneResultsState::Enter()
{
	DxPlus::Debug::SetFormatString(L"GameSceneResultsState::Enter");
}

void GameSceneResultsState::Tick(float deltaTime)
{
	DxPlus::Debug::SetFormatString(L"GameSceneResultsState::Enter");
   gameScene_.UpdateStateCamera(deltaTime, "");
}

void GameSceneResultsState::Exit()
{
	DxPlus::Debug::SetFormatString(L"GameSceneResultsState::Enter");
}
