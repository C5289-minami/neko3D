#include "ShaderTestScene.h"
#include "DxConv.h"
#include "DxPlus/DxPlus.h"
#include "DxLib.h"
#include "ResourceKeys.h"
#include "ResourceManager.h"
#include "SceneManager.h"
#include "DrawableObject.h"
#include "ModelToonRenderer.h"

#include <algorithm>
#include <cmath>
#ifndef NDEBUG
#include "imgui.h"
#endif

#include "Player.h"
#include "Sound3D.h"

DebugSceneControls ShaderTestScene::GetDebugControls()
{
	return {
		"boss_test",
		{
			MakeDebugTransformTarget("boss_test/player", "Player", player_),
			MakeDebugTransformTarget("boss_test/boss", "Boss", boss)
		},
		DebugCameraControls{
            [this] { return camera_.IsSceneViewActive(); },
			[this] { return camera_.GetPosition(); },
			[this](const Vec3& position) { camera_.SetPosition(position); },
			[this] { camera_.ResetView(); },
			[this] {

				camera_.FocusAt(player_.GetPosition());
			},
            [this] { return camera_.GetAnglesDegrees(); },
            [this](const Vec3& angles) { camera_.SetAnglesDegrees(angles); }
		},
		[this] { return debugActorsPaused_; },
		[this](bool paused) { debugActorsPaused_ = paused; },
		transformInitialStatus_
	};
}

void ShaderTestScene::Init()
{
	// 背景色
	DxLib::SetBackgroundColor(gameContext->GetBgRed(), gameContext->GetBgGreen(), gameContext->GetBgBlue());


  // 通常カメラの初期化
	const Vec3 cameraEye{ 0.0f, 250.0f, -650.0f };
	camera_.Initialize(cameraEye, { 0.0f, 100.0f, 0.0f });
	camera_.ApplyCamera();

	// トゥーン描画に必要なシェーダーと設定バッファーを初期化
	if (!modelToonRenderer_.Init())
		DxPlus::Utils::FatalError(L"modelToonRendererの初期化失敗");
	if (!shaderEffects_.Init(DxPlus::CLIENT_WIDTH, DxPlus::CLIENT_HEIGHT))
		DxPlus::Utils::FatalError(L"shaderEffectsの初期化失敗");

	// toon描画とアウトライン描画を有効にする
	shaderEffects_.GetSettings().toonEnabled = true;
	shaderEffects_.GetSettings().outlineEnabled = true;

	// FadeInを開始する
	StartFadeIn();
	

	// Objの初期化
	stage.Init();
	boss.Init();
	// プレイヤーの初期設定
	player_.Init();
	player_.Reset();
	player_.Update(0, stage);
	debugActorsPaused_ = false;
	TransformSettings::ResetToDefaults(GetDebugControls().targets, transformInitialStatus_);
	stage.ResetAlphaModel(player_.GetPosition());
}

void ShaderTestScene::Update(float deltaTime)
{
	DxLib::SetBackgroundColor(gameContext->GetBgRed(), gameContext->GetBgGreen(), gameContext->GetBgBlue());
#ifndef NDEBUG
 camera_.UpdateSceneView(deltaTime, player_.GetPosition());
	const bool actorsPaused = debugActorsPaused_ || camera_.IsSceneViewActive();
#else
	const bool actorsPaused = false;
#endif
#ifndef NDEBUG
	const bool mouseCaptured = ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureMouse;
	const bool keyboardCaptured = ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureKeyboard;
#else
	const bool mouseCaptured = false;
	const bool keyboardCaptured = false;
#endif
    const bool updateActors = !actorsPaused && !keyboardCaptured;
    if (updateActors)
    {
        player_.Update(deltaTime, stage);
        boss.Update(deltaTime);
    }

	// --- スキル入力処理 ---
	int selectedSkill = 0;
	if (DxLib::CheckHitKey(KEY_INPUT_1)) selectedSkill = 1;
	else if (DxLib::CheckHitKey(KEY_INPUT_2)) selectedSkill = 2;
	else if (DxLib::CheckHitKey(KEY_INPUT_3)) selectedSkill = 3;
	else if (DxLib::CheckHitKey(KEY_INPUT_4)) selectedSkill = 4;
	else if (DxLib::CheckHitKey(KEY_INPUT_5)) selectedSkill = 5;

	const bool skillKeyDown = selectedSkill != 0;
	if (!actorsPaused && !keyboardCaptured && skillKeyDown && !skillKeyWasDown_ && boss.GetCurrentState() != BossStateType::Attack)
	{
		BossPoseSkill* skill = nullptr;
		switch (selectedSkill)
		{
		case 1: skill = &bangSkill_; break;
		case 2: skill = &tailWhipSkill_; break;
		case 3: skill = &clapSkill_; break;
		case 4: skill = &hairBallSkill_; break;
		case 5: skill = &biteSkill_; break;
		}

		if (skill)
		{
			skill->Reset();
			boss.UseSkill(*skill);
		}
	}
	skillKeyWasDown_ = skillKeyDown;
#ifndef NDEBUG
	if (DxLib::CheckHitKey(KEY_INPUT_F1))
	{
		SetNextScene(SceneManager::GetInstance().GetScene(SceneID::Title));
		StartFadeOut();
		return;
	}
#endif

	// --- 右クリックを押している間のみカメラ操作 ---
   if (!mouseCaptured && !camera_.IsSceneViewActive() && (DxLib::GetMouseInput() & MOUSE_INPUT_RIGHT) != 0)
	{
		SetMouseDispFlag(FALSE);
		camera_.InputUpdate(deltaTime);
	}
  else SetMouseDispFlag(TRUE);
  const bool touching = Collision::UpdateModelContact(player_.GetModelObject(), boss.GetModelObject());
  if (updateActors && touching) player_.TakeDamage();
//Collision::UpdateModelContact(player_.GetModelObject(), boss.GetModelObject());


	stage.UpdateAlphaModel(deltaTime, player_);


	// --- F2 でシェーダー全体のON/OFFを切り替える ---
	static bool shaderToggleWasDown = false;
	const bool shaderToggleDown =
		DxLib::CheckHitKey(KEY_INPUT_F2) != 0;

	if (shaderToggleDown && !shaderToggleWasDown)
	{
		shaderEffects_.SetEnabled(
			!shaderEffects_.IsEnabled()
		);
	}

	shaderToggleWasDown = shaderToggleDown;
}

void ShaderTestScene::Render() const
{
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

    player_.DrawStatus();

	const int white = DxLib::GetColor(255, 255, 255);
	int fontHandle = RM().GetFont(ResourceKeys::Font_Title);
#ifndef NDEBUG
	DxPlus::Text::DrawString(L"BOSS TEST  |  F1: Back to Title",
		{ 24.0f, 24.0f }, white, DxPlus::Text::TextAlign::TOP_LEFT, { 1, 1 }, 0, fontHandle);
#endif
	DxPlus::Text::DrawString(L"1: Bang   2: Tail Whip   3: Clap   4: HairBall   5: Bite",
		{ 24.0f, 100.0f }, white, DxPlus::Text::TextAlign::TOP_LEFT, { 0.7f, 0.7f }, 0, fontHandle);
}

void ShaderTestScene::RenderDepth() const
{
	// シーンのカメラを設定する
 camera_.ApplyCamera();
	// Boneなしシェーダー
	modelToonRenderer_.SetVertexShader(ModelToonType::Model);

	stage.GetModelObject().Draw();
	boss.Draw();

	// Bone対応の頂点シェーダー
	modelToonRenderer_.SetVertexShader(ModelToonType::Bone);

	player_.Draw();

}

void ShaderTestScene::DrawScene(bool toonEnabled) const
{
	// カメラを設定する
 camera_.ApplyCamera();
	// トゥーンシェーダーを使わない場合
	if (!toonEnabled)
	{
		SetUsePixelShader(-1);
		SetUseVertexShader(-1);

		stage.Draw();
		boss.Draw();
		player_.Draw();

		return;
	}

	// トゥーン描画開始
	modelToonRenderer_.BeginPass();

	const auto drawActor = [this](const ModelObject& model, ModelToonType type)
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
	drawActor(player_.GetModelObject(), ModelToonType::Bone);
	drawActor(boss.GetModelObject(),	ModelToonType::Model);
	drawActor(stage.GetModelObject(),	ModelToonType::Model);

	modelToonRenderer_.EndPass();
}
