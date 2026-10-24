#include "GameScene.h"
#include "GameScene.h"
#include "GameContext.h"
#include "ToonSettings.h"
#include "OutlineSettings.h"
#include "CameraObjectManager.h"
#include "UIManager.h"

#include <algorithm>

#include "DxPlus/DxPlus.h"



GameScene::GameScene(GameContext* context)
	: Scene(context), stateMachine_(*this) {
}

void GameScene::Init()
{
	gameContext->Reset();

	// チュートリアルも、使いたいモデルを登録するだけで同じ判定を利用できる。
	tutorialCollision_.Clear();
	tutorialCollision_.AddModel(gameContext->GetRoom().GetRoomObject(), ModelCollision::Surface::Floor);
	tutorialCollision_.AddModel(gameContext->GetRoom().GetRoomObject(), ModelCollision::Surface::Wall);

	for (const auto& jumpStand : gameContext->GetRoom().GetJumpStands())
	{
		tutorialCollision_.AddModel(jumpStand->GetJumpStandModel(), ModelCollision::Surface::Floor);
		tutorialCollision_.AddModel(jumpStand->GetPillarModel(), ModelCollision::Surface::Pillar);
	}
	for (const auto& pillar : gameContext->GetRoom().GetPillars())
		tutorialCollision_.AddModel(*pillar, ModelCollision::Surface::Pillar);

	//for (const auto& pillar : gameContext->GetStage().GetPillarModelObjects())
	//	tutorialCollision_.AddModel(pillar, ModelCollision::Surface::Pillar);

	// トゥーン描画とアウトライン描画を有効にする
	{
		ToonSettingsManager::Load();
		OutlineSettingsManager::Load();

		// トゥーン描画に必要なシェーダーと設定バッファーを初期化
		if (!modelToonRenderer_.Init())
			DxPlus::Utils::FatalError(L"modelToonRendererの初期化失敗");
		if (!shaderEffects_.Init(DxPlus::CLIENT_WIDTH, DxPlus::CLIENT_HEIGHT))
			DxPlus::Utils::FatalError(L"shaderEffectsの初期化失敗");

		// toon描画とアウトライン描画を有効にする
		shaderEffects_.GetSettings().toonEnabled = true;
		shaderEffects_.GetSettings().outlineEnabled = true;
	}


	// camera
	{
		cameraDirector_.Reset();
		activeCameraShot_.clear();
		if (!CObjs().LoadCameras())DxPlus::Utils::FatalError(L"カメラのロードに失敗");
		if (CObjs().LoadCameras())
		{
			for (const auto& shot : CObjs().GetCameras())
			{
				cameraDirector_.RegisterShot(shot.name, shot.pose);
			}
		}
		camera_.Initialize(CObjs().GetCameraPose("Title"));
		cameraDirector_.Update(0);
	}

	// UI
	{
		// logoのUIを作成
		const DxPlus::Vec2 logoPos(DxPlus::CLIENT_WIDTH * 0.3f, DxPlus::CLIENT_HEIGHT * 0.3f);
		const std::wstring logoKey = ResourceKeys::UI_Logo;
		UIM().CreateImageUI(L"Game.Logo", logoKey, logoPos, {}, 0.5f);
		UIM().GetUI(L"Game.Logo")->SetDisplayCondition(
			[this]() -> bool
			{
				return GetCurrentGameState() == GameStateType::Title;
			}
		);

		// スタートボタンのUIを作成
		const DxPlus::Vec2 startPos(DxPlus::CLIENT_WIDTH * 0.2f, DxPlus::CLIENT_HEIGHT * 0.6f);
		const std::wstring key = ResourceKeys::UI_StartButton;
		UIM().CreateButtonUI(L"Game.StartButton", key, startPos,
			[this]() { stateMachine_.ChangeState(GameStateType::Select); },
			{ -1,0 },
			0.5f
		);
		UIM().GetUI(L"Game.StartButton")->SetDisplayCondition(
			[this]() -> bool
			{
				return GetCurrentGameState() == GameStateType::Title;
			}
		);

		// 終了ボタンのUIを作成
		const DxPlus::Vec2 exitPos(DxPlus::CLIENT_WIDTH * 0.2f, DxPlus::CLIENT_HEIGHT * 0.75f);
		const std::wstring exitKey = ResourceKeys::UI_ExitButton;
		UIM().CreateButtonUI(L"Game.ExitButton", exitKey, exitPos,
			[]() { PostQuitMessage(0); },
			{ -1,0 },
			0.5f
		);
		UIM().GetUI(L"Game.ExitButton")->SetDisplayCondition(
			[this]() -> bool
			{
				return GetCurrentGameState() == GameStateType::Title;
			}
		);
	}

	stateMachine_.Initialize();

	StartFadeIn();
}

bool GameScene::UpdateStateCamera(float deltaTime, const std::string& requestedShot)
{
	const bool transitionCompleted = cameraDirector_.Update(deltaTime);
	if (!requestedShot.empty() && requestedShot != activeCameraShot_ && !cameraDirector_.IsTransitioning())
	{
		if (cameraDirector_.PlayShot(requestedShot, 1.2f, Motion::Easing::EaseInOut))
		{
			activeCameraShot_ = requestedShot;
		}
	}
	return transitionCompleted;
}

void GameScene::FocusTutorialCamera()
{
	const auto& player = gameContext->GetPlayer();
	const auto& stage = gameContext->GetStage();

	const Vec3 playerPosition = player.GetWorldPosition();
	const Vec3 stagePosition = stage.GetWorldPosition();

	// StageからPlayerへ向かう方向を求める
	Vec3 direction = playerPosition - stagePosition;
	direction.y = 0.0f;

	const float lengthSq =
		direction.x * direction.x +
		direction.z * direction.z;

	if (lengthSq < 0.0001f)
		return;

	direction = direction.Normalized();

	Vec3 cameraPosition = stagePosition;
	// カメラの高さ

	// Playerを挟んでStageの反対側に配置する
	cameraPosition = playerPosition + direction * 300.0f;

	cameraPosition.y = playerPosition.y + 100.0f;
	

	auto pose = cameraDirector_.GetCurrentPose();
	pose.position = cameraPosition;
	pose.pitch = 0.1f; // 少し上を向く
	pose.yaw = std::atan2(-direction.x, -direction.z);
	pose.fieldOfViewDegrees = 40.0f;

	cameraDirector_.MoveTo(pose, 0.1f, Motion::Easing::EaseInOut);
}
void GameScene::Update(float deltaTime)
{
	DxLib::SetBackgroundColor(gameContext->GetBgRed(), gameContext->GetBgGreen(), gameContext->GetBgBlue());

	// Actor取得
	auto& player = gameContext->GetPlayer();
	auto& boss = gameContext->GetBoss();
	auto& stage = gameContext->GetStage();
	// モデル取得
	ModelObject& playerModel = player.GetModelObject();
	ModelObject& bossModel = boss.GetModelObject();

	// actors
	{

		// stageの中心座標を取得して、playerの更新に渡す
		const Vec3 stageCenterPosition = stage.GetPosition();

		// チュートリアルは通常移動、バトルはステージ中心を基準にした移動を使う。
		const bool tutorial = GetCurrentGameState() == GameStateType::Tutorial;
		player.SetDefMove(tutorial);
		if (tutorial)
			// player.Update(deltaTime);
			player.Update(deltaTime, tutorialCollision_);
		else
			// player.Update(deltaTime, stage, stageCenterPosition);
			player.Update(deltaTime, stage.GetCollision(), stageCenterPosition);

		stage.UpdateAlphaModel(deltaTime, player);
		boss.Update(deltaTime);

		// playerとbossの衝突判定
		const bool touching = Collision::UpdateModelContact(playerModel, bossModel);
		if (touching)
		{
			player.TakeDamage();
		}
		// 旧：部屋との接触で重力を止める処理。現在は ModelCollision の着地判定を使う。
		// const bool stageTouching = Collision::UpdateModelContact(playerModel, gameContext->GetRoom().GetModelObject());
		// if (stageTouching)
		// {
			// // プレイヤーがステージに接触している場合の処理
			// player.SetGravityEnabled(false);
			// player.SetVerticalSpeed(0.0f);
			// player.SetGround(true);
		// }


	}

	stateMachine_.Tick(deltaTime);

	gameContext->GetEffectManager().Update(deltaTime);
}

void GameScene::Render() const
{

	DxLib::ClearDrawScreen();
	if (shaderEffects_.IsEnabled())
	{
		shaderEffects_.Render(
			[this](bool toonEnabled) {
				DrawScene(toonEnabled);
			},
			[this] {
				RenderDepth();
			}
		);
	}
	else
	{
		DrawScene(false);
	}
}

void GameScene::RenderDepth() const
{
	cameraDirector_.GetActiveCamera()->ApplyPose(
		cameraDirector_.GetCurrentPose()
	);

	const auto& boss = gameContext->GetBoss();
	const auto& player = gameContext->GetPlayer();
	const auto& stage = gameContext->GetStage();
	const auto& room = gameContext->GetRoom();

	// Room本体
	modelToonRenderer_.DrawDepth(room.GetRoomObject());

	// Roomの柱
	for (const auto* pillar : room.GetPillars())
	{
		if (pillar)
		{
			modelToonRenderer_.DrawDepth(*pillar);
		}
	}

	// Roomのジャンプ台と柱
	for (const auto* jumpStand : room.GetJumpStands())
	{
		if (!jumpStand) continue;

		modelToonRenderer_.DrawDepth(
			jumpStand->GetJumpStandModel()
		);

		modelToonRenderer_.DrawDepth(
			jumpStand->GetPillarModel()
		);
	}

	// Stage
	modelToonRenderer_.DrawDepth(stage.GetModelObject());
	modelToonRenderer_.DrawDepth(stage.GetAlphaModelObject());

	for (const auto& pillar : stage.GetPillarModelObjects())
	{
		modelToonRenderer_.DrawDepth(pillar);
	}

	for (const auto& model : stage.GetStageModelObjects())
	{
		modelToonRenderer_.DrawDepth(model);
	}

	// PlayerとBoss
	modelToonRenderer_.DrawDepth(player.GetModelObject());
	modelToonRenderer_.DrawDepth(boss.GetModelObject());
}

void GameScene::DrawScene(bool toonEnabled) const
{
	cameraDirector_.GetActiveCamera()->ApplyPose(
		cameraDirector_.GetCurrentPose()
	);

	const auto& boss = gameContext->GetBoss();
	const auto& player = gameContext->GetPlayer();
	const auto& stage = gameContext->GetStage();
	const auto& effectManager = gameContext->GetEffectManager();
	const auto& grid = gameContext->GetGrid();
	const auto& room = gameContext->GetRoom();

	// トゥーンシェーダーを使わない場合
	if (!toonEnabled)
	{
		SetUsePixelShader(-1);
		SetUseVertexShader(-1);

		boss.Draw();
		stage.Draw();
		player.Draw();
		stage.DrawAlphaModel();

		effectManager.Draw();
		grid.Draw();
		room.Draw();

		return;
	}

	modelToonRenderer_.BeginPass();

	// 元のModelObjectを直接描画する
	// 親子変換はModelObject::ApplyTransform()に任せる
	const auto drawModel = [this](const ModelObject& model)
		{
			if (model.collisionHighlighted)
			{
				SetUsePixelShader(-1);
				SetUseVertexShader(-1);

				MV1SetUseOrigShader(FALSE);
				model.Draw();
				MV1SetUseOrigShader(TRUE);
			}
			else
			{
				modelToonRenderer_.Draw(model);
			}
		};

	// BossとPlayer
	drawModel(boss.GetModelObject());
	drawModel(player.GetModelObject());

	// Stage
	drawModel(stage.GetModelObject());
	drawModel(stage.GetAlphaModelObject());

	for (const auto& pillar : stage.GetPillarModelObjects())
	{
		drawModel(pillar);
	}

	for (const auto& model : stage.GetStageModelObjects())
	{
		drawModel(model);
	}

	// Room本体
	drawModel(room.GetRoomObject());

	// Roomの柱
	for (const auto* pillar : room.GetPillars())
	{
		if (pillar)
		{
			drawModel(*pillar);
		}
	}

	// Roomのジャンプ台と柱
	for (const auto* jumpStand : room.GetJumpStands())
	{
		if (!jumpStand) continue;

		drawModel(jumpStand->GetJumpStandModel());
		drawModel(jumpStand->GetPillarModel());
	}

	effectManager.Draw();
	grid.Draw();

	modelToonRenderer_.EndPass();
}

void GameScene::End()
{
	tutorialCollision_.Clear();
}
