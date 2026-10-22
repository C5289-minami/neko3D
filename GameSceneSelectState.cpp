#include "GameSceneSelectState.h"
#include "GameSceneSelectState.h"
#include "GameScene.h"
#include "GameStateMachine.h"
#include "DxPlus/Debug.h"

GameSceneSelectState::GameSceneSelectState(GameStateMachine& gameStateMachine, GameScene& gameScene)
	:GameBaseState(gameStateMachine, gameScene)
{
}

void GameSceneSelectState::Enter()
{
	DxPlus::Debug::SetFormatString(L"GameSceneSelectState::Enter");
}

void GameSceneSelectState::Tick(float deltaTime)
{
	DxPlus::Debug::SetFormatString(L"GameSceneSelectState::Enter");
   if (gameScene_.UpdateStateCamera(deltaTime, "Select"))
	{
		stateMachine_.ChangeState(GameStateType::Tutorial);
	}
}

void GameSceneSelectState::Exit()
{
	DxPlus::Debug::SetFormatString(L"GameSceneSelectState::Enter");
}
