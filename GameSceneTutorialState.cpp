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
    DxPlus::Debug::SetFormatString(
        L"GameSceneTutorialState::Enter"
    );

    auto* gameContext = gameScene_.GetGameContext();
    auto& player = gameContext->GetPlayer();

    player.SetPosition(tutorialPlayerStartPos);
    player.SetDefMove(true);

    auto& camera = gameScene_.GetTargetCamera();

    camera.Initialize(
        gameScene_.GetCameraDirector().GetCurrentPose()
    );

    // 最初のカメラ位置とPlayerの距離を記録する
    cameraPositionOffset_ =
        camera.GetPosition() - player.GetPosition();

    // 固定カメラのままPlayerを見る
    camera.SetTarget(
        player.GetPosition() + Vec3::Up() * 100.0f
    );

    targetCameraSwitched_ = false;
    tutorialCameraMoved_ = false;
}

void GameSceneTutorialState::Tick(float deltaTime)
{
    DxPlus::Debug::SetFormatString(
        L"GameSceneTutorialState::Tick"
    );

    auto& player = gameScene_.GetGameContext()->GetPlayer();

    // F2で即座にBattleへ
    if (DxLib::CheckHitKey(KEY_INPUT_F2))
    {
        stateMachine_.ChangeState(GameStateType::Battle);
        return;
    }

    // 追従開始前は、固定カメラからPlayerを見る
    if (!tutorialCameraMoved_)
    {
        gameScene_.GetTargetCamera().SetTarget(
            player.GetPosition() + Vec3::Up() * 100.0f
        );
    }

    // Tutorialカメラへの初期遷移
    if (gameScene_.UpdateStateCamera(deltaTime, "Tutorial"))
    {
        if (!targetCameraSwitched_)
        {
            gameScene_.GetCameraDirector().SwitchCamera(
                gameScene_.GetTargetCamera(),
                1.2f
            );

            targetCameraSwitched_ = true;
        }
    }

    // 一定の高さに達したら追従を開始
    if (targetCameraSwitched_
        && !tutorialCameraMoved_
        && player.GetPosition().y > tutorialPlayerStartPos.y + 50.0f)
    {
        tutorialCameraMoved_ = true;
    }

    // 追従開始後は毎フレーム、StageとPlayerからカメラ位置を再計算
    if (tutorialCameraMoved_)
    {
        gameScene_.FocusTutorialCamera();
    }
}

void GameSceneTutorialState::Exit()
{
	DxPlus::Debug::SetFormatString(L"GameSceneTutorialState::Exit");
	GameContext* gameContext = gameScene_.GetGameContext();
	auto& player = gameContext->GetPlayer();
	player.SetDefMove(false);
}
