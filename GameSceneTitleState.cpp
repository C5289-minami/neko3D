#include "GameSceneTitleState.h"
#include "GameScene.h"
#include "GameStateMachine.h"
#include "DxPlus/Debug.h"

GameSceneTitleState::GameSceneTitleState(GameStateMachine& gameStateMachine, GameScene& gameScene)
	: GameBaseState(gameStateMachine, gameScene)
{
}

void GameSceneTitleState::Enter()
{
	DxPlus::Debug::SetFormatString(L"GameSceneTitleState::Enter");
}

void GameSceneTitleState::Tick(float deltaTime)
{
	// タイトル画面の更新処理
	DxPlus::Debug::SetFormatString(L"GameSceneTitleState::Tick");
	gameScene_.UpdateStateCamera(deltaTime, "");
}

void GameSceneTitleState::Exit()
{
	DxPlus::Debug::SetFormatString(L"GameSceneTitleState::Exit");
}