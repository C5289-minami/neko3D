#include "GameSceneSelectState.h"
#include "GameScene.h"
#include "GameStateMachine.h"
#include "GameContext.h"
#include "DxPlus/Debug.h"

// Playerを三体用意、でも移動はできないやつ。
// 選択してしたキャラクターをgameContextにセットする。選択後はTutorialStateに遷移する。
namespace
{
	const Vec3 kagoPos{ -1882.3f, -783.7f, -1073.0f };
	ModelObject player1Model;
	ModelObject player2Model;
	ModelObject player3Model;
}

GameSceneSelectState::GameSceneSelectState(GameStateMachine& gameStateMachine, GameScene& gameScene)
	:GameBaseState(gameStateMachine, gameScene)
{
}

void GameSceneSelectState::Enter()
{
	DxPlus::Debug::SetFormatString(L"GameSceneSelectState::Enter");
	gameScene_.GetGameContext()->GetPlayer().SetPosition({ -2169.332763671875,317.12353515625,-1188.287109375 });
	DxLib::SetMouseDispFlag(TRUE);
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
	DxLib::SetMouseDispFlag(FALSE);
}
