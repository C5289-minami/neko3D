#include "GameSceneBattleState.h"
#include "GameScene.h"
#include "GameStateMachine.h"
#include "DxPlus/Debug.h"

GameSceneBattleState::GameSceneBattleState(GameStateMachine& gameStateMachine, GameScene& gameScene)
	: GameBaseState(gameStateMachine, gameScene)
{
}

void GameSceneBattleState::Enter()
{
	DxPlus::Debug::SetFormatString(L"GameSceneBattleState::Enter");
}

void GameSceneBattleState::Tick(float deltaTime)
{
	DxPlus::Debug::SetFormatString(L"GameSceneBattleState::Tick");
   if (gameScene_.UpdateStateCamera(deltaTime, "Battle"))
	{
		stateMachine_.ChangeState(GameStateType::Title);
	}
}

void GameSceneBattleState::Exit()
{
	DxPlus::Debug::SetFormatString(L"GameSceneBattleState::Exit");
}
