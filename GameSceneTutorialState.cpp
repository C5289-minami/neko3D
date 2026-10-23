#include "GameSceneTutorialState.h"
#include "GameScene.h"
#include "GameStateMachine.h"
#include "DxPlus/Debug.h"
#include "GameContext.h"

namespace
{
	const Vec3 tutorialPlayerStartPos{ -1882.3f, -783.7f, -1373.0f };
}


GameSceneTutorialState::GameSceneTutorialState(GameStateMachine& gameStateMachine, GameScene& gameScene)
	: GameBaseState(gameStateMachine, gameScene)
{
}

void GameSceneTutorialState::Enter()
{
	DxPlus::Debug::SetFormatString(L"GameSceneTutorialState::Enter");
	GameContext* gameContext = gameScene_.GetGameContext();

	// player‚Ì‰ŠúˆÊ’u‚ðÝ’è
	auto& player = gameContext->GetPlayer();
	player.SetPosition(tutorialPlayerStartPos);
	player.SetDefMove(true);
	gameScene_.GetTargetCamera().Initialize(gameScene_.GetCameraDirector().GetCurrentPose());
	cameraPositionOffset_ = gameScene_.GetTargetCamera().GetPosition() - player.GetPosition();
	gameScene_.GetTargetCamera().SetTarget(player.GetPosition() + Vec3::Up() * 100.0f);
	targetCameraSwitched_ = false;

}

void GameSceneTutorialState::Tick(float deltaTime)
{
	DxPlus::Debug::SetFormatString(L"GameSceneTutorialState::Tick");
	const bool battleRequested = DxLib::CheckHitKey(KEY_INPUT_F2);
	if (battleRequested)
	{
		stateMachine_.ChangeState(GameStateType::Battle);
	}
	else
	{
		gameScene_.FocusTutorialCamera(deltaTime, cameraPositionOffset_);
	}

	const std::string requestedShot = battleRequested ? "Battle" : "Tutorial";
	if (gameScene_.UpdateStateCamera(deltaTime, requestedShot))
	{
		if (battleRequested)
		{
			stateMachine_.ChangeState(GameStateType::Title);
		}
		else if (!targetCameraSwitched_)
		{
			gameScene_.GetCameraDirector().SwitchCamera(gameScene_.GetTargetCamera(), 1.2f);
			targetCameraSwitched_ = true;
		}
	}
}

void GameSceneTutorialState::Exit()
{
	DxPlus::Debug::SetFormatString(L"GameSceneTutorialState::Exit");
	GameContext* gameContext = gameScene_.GetGameContext();
	auto& player = gameContext->GetPlayer();
	player.SetDefMove(false);
}
