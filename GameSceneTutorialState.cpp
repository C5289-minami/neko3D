#include "GameSceneTutorialState.h"
#include "GameScene.h"
#include "GameStateMachine.h"
#include "DxPlus/Debug.h"
#include "Consts.h"
#include "GameContext.h"
#include "UIManager.h"
#include "ResourceKeys.h"
#include <cmath>
#include <algorithm>

namespace
{
	const Vec3 tutorialPlayerStartPos{ -1882.3f, -783.7f, -1373.0f };
	struct task
	{
		TutorialType type;
		std::wstring taskName;
		DxPlus::Vec2 stampPosition;
		float requiredProgress{ 1.0f };
	};
	task tasks[] =
	{
		{ TutorialType::Move,		L"3秒移動する",			DxPlus::Vec2(0.0f, -452.5f + 222.0f),3.0f },
		{ TutorialType::Jump,		L"ジャンプする",		DxPlus::Vec2(0.0f, -452.5f + 331.0f) },
		{ TutorialType::Attack,		L"攻撃する",			DxPlus::Vec2(0.0f, -452.5f + 425.0f) },
		{ TutorialType::DoubleJump,	L"二段ジャンプする",	DxPlus::Vec2(0.0f, -452.5f + 535.0f) },
		{ TutorialType::Climb,		L"柱を登る",			DxPlus::Vec2(0.0f, -452.5f + 661.0f) },
		{ TutorialType::StandOnStage,	L"上に行く",		DxPlus::Vec2(0.0f, -452.5f + 776.0f) }
	};


	constexpr float TwoPi = 6.28318530718f;
}

GameSceneTutorialState::GameSceneTutorialState(GameStateMachine& gameStateMachine, GameScene& gameScene)
	: GameBaseState(gameStateMachine, gameScene)
{
}

void GameSceneTutorialState::Enter()
{
	DxPlus::Debug::SetFormatString(L"GameSceneTutorialState::Enter");

	auto* player = gameScene_.GetGameContext()->GetPlayer();
	player->SetStartPosition(tutorialPlayerStartPos);
	player->SetDefMove(true);

	auto* skip = UIM().CreateImageUI(
		L"Tutorial.Skip", ResourceKeys::TutorialSkip,
		{ DxPlus::CLIENT_WIDTH - 155.0f, DxPlus::CLIENT_HEIGHT - 80.0f });
	if (skip)
	{
		skip->SetScale(0.8f);
		skip->SetZOrder(100);
		skip->SetTrue();
	}

	// チェックリスト初期化
	auto& checklist = gameScene_.GetTutorialChecklist();

	gameScene_.GetTutorialTaskManager().Init(checklist);

	for (const auto& t : tasks)
	{
		gameScene_.GetTutorialTaskManager().AddTask(t.type, t.taskName, t.stampPosition, t.requiredProgress);
	}


	// カメラを初期位置に設定
	auto& camera = gameScene_.GetTargetCamera();
	camera.SetTarget(player->GetPosition() + Vec3::Up() * cameraPlayerHeightOffset);

	targetCameraSwitched_ = false;
	cameraState_ = TutorialCameraState::LookPlayer;

	// PlayerStateMachineの通知をTaskManagerに接続
	player->GetStateMachine().SetEventCallback(
		[this](PlayerActionEvent event)
		{
			gameScene_.GetTutorialTaskManager().OnPlayerAction(event);
		}
	);
}

void GameSceneTutorialState::Tick(float deltaTime)
{
	DxPlus::Debug::SetFormatString(L"GameSceneTutorialState::Tick");

	auto& director = gameScene_.GetCameraDirector();
	auto* player = gameScene_.GetGameContext()->GetPlayer();
	auto& taskManager = gameScene_.GetTutorialTaskManager();

	// =========================================
	// 移動チュートリアル
	// =========================================
	
	taskManager.Update(deltaTime);

	// Walk状態の間だけ時間を加算
	if (player->GetCurrentState() == PlayerStateType::Walk)
	{
		taskManager.AddProgress(TutorialType::Move, deltaTime);
	}

	// =========================================
	// カメラ
	// =========================================

	const bool transitionCompleted =
		gameScene_.UpdateStateCamera(deltaTime, "Tutorial");

	// 初期ショットの遷移完了後、TargetCameraへ切り替える
	if (transitionCompleted && !targetCameraSwitched_)
	{
		director.SwitchCamera(
			gameScene_.GetTargetCamera(),
			1.2f);
		auto& camera = gameScene_.GetTargetCamera();
		camera.Initialize(gameScene_.GetCameraDirector().GetCurrentPose());

		// チェックリストを表示する
		auto& checklist = gameScene_.GetTutorialChecklist();
		checklist.SetVisible(true);
		// TargetCameraに切り替えたことを記録する
		targetCameraSwitched_ = true;
		// Playerの影を描画する
		gameScene_.SetDrawPlayerShadow(true);

	}

	switch (cameraState_)
	{
	case TutorialCameraState::LookPlayer:
		// 固定カメラからPlayerを見る
		gameScene_.GetTargetCamera().SetTarget(
			player->GetPosition() + Vec3::Up() * cameraPlayerHeightOffset);
		break;

	case TutorialCameraState::EnterMove:
		// MoveToが完了したフレームで追従へ
		if (transitionCompleted)
		{
			cameraState_ = TutorialCameraState::FollowPlayer;
		}
		break;

	case TutorialCameraState::FollowPlayer:
		FocusTutorialCamera(deltaTime);
		break;
	}

	// =========================================
	// 当たり判定
	// =========================================

	// GameScene が管理する球体設定を取得
	const auto& settings = gameScene_.GetTutorialCollisionSettings();

	auto& stage = gameScene_.GetGameContext()->GetStage();

	const Vec3 playerCenter =
		player->GetWorldPosition() + settings.playerOffset;

	const Vec3 stageCenter =
		stage.GetWorldPosition() + settings.stageOffset;

	// カメラ追従開始判定
	const bool touchingClimb = Collision::IsHitSphereSphere(
		playerCenter,
		settings.playerRadius,
		stageCenter,
		settings.climbRadius
	);


	if (touchingClimb &&
		cameraState_ == TutorialCameraState::LookPlayer)
	{
		cameraState_ = TutorialCameraState::EnterMove;

		director.MoveTo(
			GetTutorialCameraPose(),
			1.0f,
			Motion::Easing::EaseInOut);
	}

	// Battleへの遷移判定
	const bool touchingStage = Collision::IsHitSphereSphere(
		playerCenter,
		settings.playerRadius,
		stageCenter,
		settings.stageRadius
	);

	if (touchingStage &&
		stage.GetPosition().y < playerCenter.y + settings.playerRadius)
	{
		stateMachine_.ChangeState(GameStateType::Battle);
		taskManager.Complete(TutorialType::StandOnStage);// 立った瞬間に達成
	}
}

void GameSceneTutorialState::Exit()
{
	UIM().RemoveUI(L"Tutorial.Skip");

	DxPlus::Debug::SetFormatString(L"GameSceneTutorialState::Exit");

	auto* player = gameScene_.GetGameContext()->GetPlayer();

	player->SetDefMove(false);

	gameScene_.SetDrawPlayerShadow(true);
}

void GameSceneTutorialState::FocusTutorialCamera(float deltaTime)
{
	auto* gameContext = gameScene_.GetGameContext();
	const auto* player = gameContext->GetPlayer();
	const auto& stage = gameContext->GetStage();

	const Vec3 playerPos = player->GetWorldPosition();
	const Vec3 stagePos = stage.GetWorldPosition();

	// ---------------------------------------------------------
	// ステージから見てプレイヤーの真後ろを計算
	// ---------------------------------------------------------
	Vec3 stageToPlayer = playerPos - stagePos;
	stageToPlayer.y = 0.0f;
	if (stageToPlayer.LengthSq() < Const::EPS) return;
	stageToPlayer = stageToPlayer.Normalized();

	// カメラが見る場所（プレイヤーの少し上 )
	const Vec3 lookPoint = playerPos + Vec3::Up() * cameraPlayerHeightOffset;

	// カメラが目指す場所（プレイヤーから後ろに cameraDistance 離れた位置）
	const Vec3 desiredPosition = lookPoint + stageToPlayer * cameraDistance;

	// ---------------------------------------------------------
	// カメラ移動
	// ---------------------------------------------------------
	auto& director = gameScene_.GetCameraDirector();
	Camera::CameraPose pose = director.GetCurrentPose();

	const Vec3 diff = desiredPosition - pose.position;
	const float distance = std::sqrt(diff.LengthSq());

	if (distance > Const::EPS)
	{
		float speed = cameraFollowSpeed;

		// 一定距離以上離れたら、離れた分だけスピードを2乗で急行する
		if (distance > cameraMaxLag)
		{
			const float overRatio = distance / cameraMaxLag;
			speed *= (overRatio * overRatio); // 乗算で速度をあげる　
		}

		const float step = std::min(speed * deltaTime, distance);
		pose.position += diff * (step / distance);
	}

	// ---------------------------------------------------------
	// カメラの向き調整
	// ---------------------------------------------------------
	const Vec3 viewDir = lookPoint - pose.position;
	const float targetYaw = std::atan2(viewDir.x, viewDir.z);

	const float turnBlend = std::clamp(cameraTurnSpeed * deltaTime, 0.0f, 1.0f);
	pose.yaw += std::remainder(targetYaw - pose.yaw, TwoPi) * turnBlend;

	pose.pitch = cameraPitch;
	pose.fieldOfViewDegrees = cameraFov;

	director.SetPoseImmediate(pose);
}

Camera::CameraPose GameSceneTutorialState::GetTutorialCameraPose()
{
	auto& context = *gameScene_.GetGameContext();
	const auto* player = context.GetPlayer();
	const auto& stage = context.GetStage();

	const Vec3 playerPos = player->GetWorldPosition();
	const Vec3 stagePos = stage.GetWorldPosition();

	Vec3 stageToPlayer = playerPos - stagePos;
	stageToPlayer.y = 0.0f;

	if (stageToPlayer.LengthSq() < Const::EPS)
		stageToPlayer = Vec3(0.0f, 0.0f, 1.0f);
	else
		stageToPlayer = stageToPlayer.Normalized();

	const Vec3 lookPoint = playerPos + Vec3::Up() * cameraPlayerHeightOffset;

	Camera::CameraPose pose{};

	pose.position = lookPoint + stageToPlayer * cameraDistance;

	const Vec3 viewDir = lookPoint - pose.position;

	pose.yaw = std::atan2(viewDir.x, viewDir.z);
	pose.pitch = cameraPitch;
	pose.fieldOfViewDegrees = cameraFov;

	return pose;
}
