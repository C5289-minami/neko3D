#include "GameScene.h"
#include "GameContext.h"
#include "ToonSettings.h"
#include "OutlineSettings.h"
#include "CameraObjectManager.h"
#include "UIManager.h"
#include <algorithm>
#include "DxPlus/DxPlus.h"
#include "Collision.h"
#include "DxConv.h"

#ifndef NDEBUG
#include "imgui.h"
#endif

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

	// チュートリアルでもStageの柱を判定する。
	for (const auto& pillar : gameContext->GetStage().GetPillarModelObjects())
		tutorialCollision_.AddModel(pillar, ModelCollision::Surface::Pillar);

	for (const auto* pillar : gameContext->GetRoom().GetPillars())
		tutorialCollision_.AddModel(*pillar, ModelCollision::Surface::Pillar);

	// ジャンプ台の床と柱を、それぞれの用途で登録する。
	for (const auto* stand : gameContext->GetRoom().GetJumpStands())
	{
		tutorialCollision_.AddModel(stand->GetJumpStandModel(), ModelCollision::Surface::Floor);
		tutorialCollision_.AddModel(stand->GetPillarModel(), ModelCollision::Surface::Pillar);
	}
	tutorialCollision_.AddModel(gameContext->GetStage().GetModelObject(), ModelCollision::Surface::Floor);

	// キャラクター選択画面のプレビューを表示する
	gameContext->SetSelectPreviewVisible(true);

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
		if (!CObjs().LoadCameras())
			DxPlus::Utils::FatalError(L"カメラのロードに失敗");
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


void GameScene::Update(float deltaTime)
{
	DxLib::SetBackgroundColor(gameContext->GetBgRed(), gameContext->GetBgGreen(), gameContext->GetBgBlue());

	// BattleとTutorial以外は本編Playerの移動・衝突判定を止める
	if (GetCurrentGameState() != GameStateType::Tutorial &&
		GetCurrentGameState() != GameStateType::Battle)
	{
		stateMachine_.Tick(deltaTime);
		gameContext->GetEffectManager().Update(deltaTime);
		return;

	}
	// Actor取得
	auto* player = gameContext->GetPlayer();
	auto& boss = gameContext->GetBoss();
	auto& stage = gameContext->GetStage();
	// モデル取得
	ModelObject& playerModel = player->GetModelObject();
	ModelObject& bossModel = boss.GetModelObject();

	// actors
	{

		// stageの中心座標を取得して、playerの更新に渡す
		const Vec3 stageCenterPosition = stage.GetPosition();

		// チュートリアルは通常移動、バトルはステージ中心を基準にした移動を使う。
		const bool tutorial = GetCurrentGameState() == GameStateType::Tutorial;
		player->SetDefMove(tutorial);
		if (tutorial)
			// player.Update(deltaTime);
			player->Update(deltaTime, tutorialCollision_);
		else
			// player.Update(deltaTime, stage, stageCenterPosition);
			player->Update(deltaTime, stage.GetCollision(), stageCenterPosition);

		stage.UpdateAlphaModel(deltaTime, *player);
		boss.Update(deltaTime);
		// playerとbossの衝突判定
		//const bool touching = Collision::UpdateModelContact(playerModel, bossModel);　これなし
		//if (touching)
		//{
		//	player.TakeDamage();
		//}





	}


	stateMachine_.Tick(deltaTime);

	if (GetCurrentGameState() == GameStateType::Tutorial)
	{
		tutorialChecklist_.Update(deltaTime);
	}

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

void GameScene::DrawImGui()
{
#ifndef NDEBUG
	ImGui::Begin("GameScene Debug");

	// clear
	if (ImGui::Button("Clear"))
	{
		stateMachine_.ChangeState(GameStateType::Clear);
	}
	if (ImGui::Button("GameOver"))
	{
		stateMachine_.ChangeState(GameStateType::GameOver);
	}

	ImGui::End();
#endif
}

void GameScene::RenderDepth() const
{
	cameraDirector_.ApplyRenderPose();

	const auto& boss = gameContext->GetBoss();
	const auto* player = gameContext->GetPlayer();
	const auto& stage = gameContext->GetStage();
	const auto& room = gameContext->GetRoom();

	// RoomObj
	for (const auto* allRoom : room.GetAllModelObjects())
	{
		modelToonRenderer_.DrawDepth(*allRoom);
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
	for (const auto& preview :
		gameContext->GetSelectPreviewPlayers())
	{
		modelToonRenderer_.DrawDepth(preview.GetModelObject());
	}

	modelToonRenderer_.DrawDepth(boss.GetModelObject());
}

void GameScene::DrawTutorialCollisionSpheres() const
{
#ifdef _DEBUG
	const auto& settings = tutorialCollisionSettings_;

	if (!settings.drawSpheres ||
		GetCurrentGameState() != GameStateType::Tutorial)
	{
		return;
	}

	const auto* player = gameContext->GetPlayer();
	const auto& stage = gameContext->GetStage();

	const Vec3 playerCenter =
		player->GetWorldPosition() + settings.playerOffset;

	const Vec3 stageCenter =
		stage.GetWorldPosition() + settings.stageOffset;

	// Collision共通のSphere型を使う
	const Collision::Sphere playerSphere{
		playerCenter, settings.playerRadius
	};

	const Collision::Sphere stageSphere{
		stageCenter, settings.stageRadius
	};

	const Collision::Sphere climbSphere{
		stageCenter, settings.climbRadius
	};

	// Player：緑
	Collision::DrawDebugSphere(
		playerSphere, DxLib::GetColor(0, 255, 0));

	// Battle遷移判定：赤
	Collision::DrawDebugSphere(
		stageSphere, DxLib::GetColor(255, 0, 0));

	// カメラ追従開始判定：青
	Collision::DrawDebugSphere(
		climbSphere, DxLib::GetColor(0, 100, 255));
#endif
}

void GameScene::DrawScene(bool toonEnabled) const
{
	cameraDirector_.ApplyRenderPose();

	const auto& boss = gameContext->GetBoss();
	const auto* player = gameContext->GetPlayer();
	const auto& stage = gameContext->GetStage();
	const auto& effectManager = gameContext->GetEffectManager();
	const auto& grid = gameContext->GetGrid();
	const auto& room = gameContext->GetRoom();

	const auto& selectPlayers =
		gameContext->GetSelectPreviewPlayers();

	// トゥーンシェーダーを使わない場合
	if (!toonEnabled)
	{
		SetUsePixelShader(-1);
		SetUseVertexShader(-1);

		player->Draw();
		boss.Draw();
		stage.Draw();


		for (const auto& preview : selectPlayers)
		{
			preview.Draw();
		}



		stage.DrawAlphaModel();

		effectManager.Draw();
		grid.Draw();
		room.Draw();


		DrawTutorialCollisionSpheres();
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

	// Room
	for (const auto* allRoom : room.GetAllModelObjects())
	{
		drawModel(*allRoom);
	}


	for (const auto& preview : selectPlayers)
	{
		drawModel(preview.GetModelObject());
	}


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
	// BossとPlayer
	drawModel(boss.GetModelObject());
	if (drawPlayerShadow_)
		DrawPlayerShadow();
	drawModel(player->GetModelObject());


	effectManager.Draw();
	modelToonRenderer_.EndPass();
	effectManager.Draw();
	player->GetStateMachine().Draw();
	DrawTutorialCollisionSpheres();
}

void GameScene::End()
{
	tutorialCollision_.Clear();
}

void GameScene::DrawPlayerShadow() const
{
	const auto* player = gameContext->GetPlayer();
	const auto& stage = gameContext->GetStage();

	// 状態に応じて使用するCollisionを選ぶ
	const ModelCollision* collision = nullptr;

	if (GetCurrentGameState() == GameStateType::Tutorial)
		collision = &tutorialCollision_;
	else if (GetCurrentGameState() == GameStateType::Battle)
		collision = &stage.GetCollision();

	if (!collision) return;

	// 床へのRaycast
	const Vec3 playerPos = player->GetWorldPosition();

	// Playerの後方へ30移動
	const float yaw = player->GetYaw();
	const Vec3 backward(
		-sinf(yaw) * 30.0f,
		0.0f,
		-cosf(yaw) * 30.0f
	);

	const Vec3 shadowPos = playerPos + backward;

	Physics::RayHit hit{};
	if (!collision->RaycastDown(
		shadowPos + Vec3::Up() * 50.0f,
		500.0f,
		hit))
	{
		return;
	}

	// 床の位置を画面座標に変換
	const VECTOR screenPos = ConvWorldPosToScreenPos(
		DxConv::ToVECTOR(hit.point));

	const int x = static_cast<int>(screenPos.x);
	const int y = static_cast<int>(screenPos.y);

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);

	DrawOval(
		x, y,
		80, 18,
		GetColor(0, 0, 0),
		TRUE);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}
