#include "GameScene.h"
#include "GameContext.h"
#include "ToonSettings.h"
#include "OutlineSettings.h"
#include "CameraObjectManager.h"

void GameScene::Init()
{
	gameContext->Reset();

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
		camera_.Initialize(CObjs().GetCameraPose("Title"));
		
		
		cameraDirector_.Reset();

		cameraDirector_.PlayShot("Select", 0.8f, Motion::Easing::EaseInOut);
	}

	StartFadeIn();
}

void GameScene::Update(float deltaTime)
{
	DxLib::SetBackgroundColor(gameContext->GetBgRed(), gameContext->GetBgGreen(), gameContext->GetBgBlue());

	{
		auto& player = gameContext->GetPlayer();
		auto& boss = gameContext->GetBoss();
		auto& stage = gameContext->GetStage();
		const Vec3 stageCenterPosition = stage.GetPosition();


		player.Update(deltaTime, stage, stageCenterPosition);
		stage.UpdateAlphaModel(deltaTime, player);
		boss.Update(deltaTime);

		// playerの位置
		ModelObject playerModel = player.GetModelObject().WithParentTransform(
			player.GetPosition(), player.GetRotation());
		
		// playerとbossの衝突判定
		const bool touching = Collision::UpdateModelContact(playerModel, boss.GetModelObject());
		player.GetModelObject().collisionHighlighted = playerModel.collisionHighlighted;
		if (touching)
		{
			player.TakeDamage();
		}

		if (cameraDirector_.Update(deltaTime))
		{
			cameraDirector_.PlayShot("Tutorial", 1.2f, Motion::Easing::EaseInOut);
		}

		gameContext->GetEffectManager().Update(deltaTime);
	}
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
	cameraDirector_.GetActiveCamera()->ApplyPose(cameraDirector_.GetCurrentPose());

	const auto& boss = gameContext->GetBoss();
	const auto& player = gameContext->GetPlayer();
	const auto& stage = gameContext->GetStage();
	const auto& effectManager = gameContext->GetEffectManager();

	// Boneなしシェーダー
	modelToonRenderer_.SetVertexShader(ModelType::Model);

	boss.Draw();
	stage.Draw();
	stage.DrawAlphaModel();
	gameContext->GetRoom().Draw();

	// Bone対応の頂点シェーダー
	modelToonRenderer_.SetVertexShader(ModelType::Bone);

	player.Draw();


}

void GameScene::DrawScene(bool toonEnabled) const
{
	cameraDirector_.GetActiveCamera()->ApplyPose(cameraDirector_.GetCurrentPose());

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

	// トゥーン描画開始
	modelToonRenderer_.BeginPass();

	const auto drawActor = [this](const ModelObject& model, ModelType type)
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
				modelToonRenderer_.Draw(model, type);
			}
		};
	const ModelObject playerModel = player.GetModelObject().WithParentTransform(
		player.GetPosition(), player.GetRotation());
	drawActor(playerModel, ModelType::Bone);
	// bossのモデルにboss自体の位置と回転を適用して描画する
	drawActor(boss.GetModelObject().WithParentTransform(
		boss.GetPosition(), boss.GetRotation()), ModelType::Model);
	drawActor(stage.GetModelObject().WithParentTransform(
		stage.GetPosition(), stage.GetRotation()), ModelType::Model);
	drawActor(stage.GetAlphaModelObject(), ModelType::Model);
	for (const auto& pillar : stage.GetPillarModelObjects())
	{
		drawActor(pillar.WithParentTransform(stage.GetPosition(), stage.GetRotation()), ModelType::Model);
	}
	for (const auto& model : stage.GetStageModelObjects())
	{
		drawActor(model.WithParentTransform(stage.GetPosition(), stage.GetRotation()), ModelType::Model);
	}
	effectManager.Draw();
	grid.Draw();

	drawActor(room.GetModelObject(), ModelType::Model);
	modelToonRenderer_.EndPass();
}