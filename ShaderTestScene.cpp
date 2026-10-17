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
#include "ToonSettings.h"
#include "OutlineSettings.h"


void ShaderTest::Init()
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
	gameContext->GetStage().Init();
	gameContext->GetBoss().Init();
	// プレイヤーの初期設定
	gameContext->GetPlayer().Init();
	gameContext->GetPlayer().Reset();
	gameContext->GetPlayer().Update(0, gameContext->GetStage());
	debugActorsPaused_ = false;
	gameContext->GetStage().ResetAlphaModel(gameContext->GetPlayer().GetPosition());
}

void ShaderTest::Update(float deltaTime)
{
	DxLib::SetBackgroundColor(gameContext->GetBgRed(), gameContext->GetBgGreen(), gameContext->GetBgBlue());
#ifndef NDEBUG
	camera_.UpdateSceneView(deltaTime, gameContext->GetPlayer().GetPosition());
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
		gameContext->GetPlayer().Update(deltaTime, gameContext->GetStage());
		gameContext->GetBoss().Update(deltaTime);
	}

	// --- スキル入力処理 ---
	int selectedSkill = 0;
	if (DxLib::CheckHitKey(KEY_INPUT_1)) selectedSkill = 1;
	else if (DxLib::CheckHitKey(KEY_INPUT_2)) selectedSkill = 2;
	else if (DxLib::CheckHitKey(KEY_INPUT_3)) selectedSkill = 3;
	else if (DxLib::CheckHitKey(KEY_INPUT_4)) selectedSkill = 4;
	else if (DxLib::CheckHitKey(KEY_INPUT_5)) selectedSkill = 5;

	const bool skillKeyDown = selectedSkill != 0;
	if (!actorsPaused && !keyboardCaptured && skillKeyDown && !skillKeyWasDown_ && gameContext->GetBoss().GetCurrentState() != BossStateType::Attack)
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
			gameContext->GetBoss().UseSkill(*skill);
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
		camera_.Update(deltaTime);
	}
	else SetMouseDispFlag(TRUE);
	auto& player = gameContext->GetPlayer();
	ModelObject playerModel = player.GetModelObject().WithParentTransform(
		player.GetPosition(), player.GetRotation());
	const bool touching = Collision::UpdateModelContact(playerModel, gameContext->GetBoss().GetModelObject());
	player.GetModelObject().collisionHighlighted = playerModel.collisionHighlighted;
	if (updateActors && touching) gameContext->GetPlayer().TakeDamage();


	gameContext->GetStage().UpdateAlphaModel(deltaTime, gameContext->GetPlayer());


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

void ShaderTest::Render() const
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

	gameContext->GetPlayer().DrawStatus();

	const int white = DxLib::GetColor(255, 255, 255);
	int fontHandle = RM().GetFont(ResourceKeys::Font_Title);
#ifndef NDEBUG
	DxPlus::Text::DrawString(L"BOSS TEST  |  F1: Back to Title",
		{ 24.0f, 24.0f }, white, DxPlus::Text::TextAlign::TOP_LEFT, { 1, 1 }, 0, fontHandle);
#endif
	DxPlus::Text::DrawString(L"1: Bang   2: Tail Whip   3: Clap   4: HairBall   5: Bite",
		{ 24.0f, 100.0f }, white, DxPlus::Text::TextAlign::TOP_LEFT, { 0.7f, 0.7f }, 0, fontHandle);
}
void ShaderTest::DrawImGui()
{
#ifndef NDEBUG
	// Outline Settings
	{
		ImGui::Begin("Outline Settings");

		ImGui::ColorEdit3(
			u8"アウトライン色",
			g_outlineSettings.outlineColor
		);

		ImGui::SliderFloat(
			u8"アウトライン太さ",
			&g_outlineSettings.radius,
			0.0f,
			5.0f
		);

		ImGui::SliderFloat(
			u8"深度強度",
			&g_outlineSettings.depthStrength,
			0.0f,
			50.0f
		);

		ImGui::SliderFloat(
			u8"法線強度",
			&g_outlineSettings.normalStrength,
			0.0f,
			20.0f
		);

		ImGui::SliderFloat(
			u8"輪郭しきい値",
			&g_outlineSettings.threshold,
			0.0f,
			1.0f
		);


		if (ImGui::Button(u8"保存"))
		{
			OutlineSettingsManager::Save();
		}

		ImGui::SameLine();

		if (ImGui::Button(u8"読み込み"))
		{
			OutlineSettingsManager::Load();
		}

		ImGui::SameLine();

		if (ImGui::Button(u8"初期値に戻す"))
		{
			OutlineSettingsManager::Reset();
		}

		ImGui::End();
	}
	// トゥーン設定
	{
		ImGui::Begin("Toon Settings");

		// =========================
		// ライト設定
		// =========================
		static bool lightAngleInitialized = false;
		static float lightAzimuth = 0.0f;
		static float lightElevation = 45.0f;

		constexpr float RadToDeg = 180.0f / DX_PI_F;
		constexpr float DegToRad = DX_PI_F / 180.0f;

		// 初回に現在のライト方向を角度へ変換
		if (!lightAngleInitialized)
		{
			const VECTOR direction = DxLib::GetLightDirection();

			const float length = std::sqrt(
				direction.x * direction.x +
				direction.y * direction.y +
				direction.z * direction.z
			);

			if (length > 0.001f)
			{
				const float x = -direction.x / length;
				const float y = -direction.y / length;
				const float z = -direction.z / length;

				lightAzimuth = std::atan2(x, z) * RadToDeg;
				lightElevation =
					std::asin(std::clamp(y, -1.0f, 1.0f)) * RadToDeg;
			}

			lightAngleInitialized = true;
		}

		ImGui::Text(u8"ライトの向き");

		bool lightChanged = ImGui::SliderFloat(
			u8"方位角",
			&lightAzimuth,
			-180.0f,
			180.0f,
			u8"%.1f 度"
		);

		lightChanged |= ImGui::SliderFloat(
			u8"高さ角",
			&lightElevation,
			-89.0f,
			89.0f,
			u8"%.1f 度"
		);

		if (lightChanged)
		{
			const float azimuth = lightAzimuth * DegToRad;
			const float elevation = lightElevation * DegToRad;

			const float x = std::cos(elevation) * std::sin(azimuth);
			const float y = std::sin(elevation);
			const float z = std::cos(elevation) * std::cos(azimuth);

			DxLib::SetLightDirection(VGet(-x, -y, -z));
		}

		// Toonシェーダー用のライト方向を同期
		const VECTOR currentLight = DxLib::GetLightDirection();

		g_toonSettings.LightDirection[0] = currentLight.x;
		g_toonSettings.LightDirection[1] = currentLight.y;
		g_toonSettings.LightDirection[2] = currentLight.z;
		g_toonSettings.LightDirection[3] = 0.0f;

		ImGui::Separator();

		// =========================
		// Toonの影設定
		// =========================
		ImGui::ColorEdit3(
			u8"影色",
			g_toonSettings.shadowColor
		);

		ImGui::SliderFloat(
			u8"明るい境界",
			&g_toonSettings.thresholds[0],
			0.0f, 1.0f
		);

		ImGui::SliderFloat(
			u8"通常境界",
			&g_toonSettings.thresholds[1],
			0.0f, 1.0f
		);

		ImGui::SliderFloat(
			u8"暗い境界",
			&g_toonSettings.thresholds[2],
			0.0f, 1.0f
		);

		float shadowDarkness = 1.0f - g_toonSettings.shadow[0];

		if (ImGui::SliderFloat(
			u8"影の暗さ",
			&shadowDarkness,
			0.0f, 1.0f))
		{
			g_toonSettings.shadow[0] = 1.0f - shadowDarkness;
		}

		ImGui::SliderFloat(
			u8"影色の強さ",
			&g_toonSettings.shadow[1],
			0.0f, 1.0f
		);

		if (ImGui::Button(u8"保存"))
		{
			ToonSettingsManager::Save();
		}

		ImGui::SameLine();

		if (ImGui::Button(u8"読み込み"))
		{
			if (ToonSettingsManager::Load())
			{
				lightAngleInitialized = false;
			}
		}

		ImGui::SameLine();

		if (ImGui::Button(u8"初期値に戻す"))
		{
			ToonSettingsManager::Reset();
		}


		ImGui::Separator();
		ImGui::Text(u8"ハーフトーン設定");

		// ハーフトーン ON/OFF
		bool halftoneEnabled = g_toonSettings.shadow[2] > 0.5f;

		if (ImGui::Checkbox(
			u8"影をトーンで表現",
			&halftoneEnabled))
		{
			g_toonSettings.shadow[2] =
				halftoneEnabled ? 1.0f : 0.0f;
		}

		// トーンの色
		ImGui::ColorEdit3(
			u8"トーンの色",
			g_toonSettings.halftoneColor
		);

		// 大きさ：数値が大きいほどドットが大きく、間隔も広がる
		ImGui::SliderFloat(
			u8"トーンの大きさ",
			&g_toonSettings.halftoneSettings[0],
			3.0f,
			20.0f,
			u8"%.1f px"
		);

		// 不透明度：0 = 透明、1 = 完全に不透明
		ImGui::SliderFloat(
			u8"トーンの不透明度",
			&g_toonSettings.halftoneSettings[1],
			0.0f,
			1.0f,
			u8"%.2f"
		);

		ImGui::End();
	}
#endif
}
void ShaderTest::RenderDepth() const
{
	// シーンのカメラを設定する
	camera_.ApplyCamera();
	// Boneなしシェーダー
	modelToonRenderer_.SetVertexShader(ModelType::Model);

	gameContext->GetStage().GetModelObject().Draw();
	gameContext->GetBoss().Draw();

	// Bone対応の頂点シェーダー
	modelToonRenderer_.SetVertexShader(ModelType::Bone);

	gameContext->GetPlayer().Draw();

}

void ShaderTest::DrawScene(bool toonEnabled) const
{
	// カメラを設定する
	camera_.ApplyCamera();
	// トゥーンシェーダーを使わない場合
	if (!toonEnabled)
	{
		SetUsePixelShader(-1);
		SetUseVertexShader(-1);

		gameContext->GetStage().Draw();
		gameContext->GetBoss().Draw();
		gameContext->GetPlayer().Draw();

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
	const auto& player = gameContext->GetPlayer();
	const ModelObject playerModel = player.GetModelObject().WithParentTransform(
		player.GetPosition(), player.GetRotation());
	drawActor(playerModel, ModelType::Bone);
	drawActor(gameContext->GetBoss().GetModelObject(), ModelType::Model);
	drawActor(gameContext->GetStage().GetModelObject(), ModelType::Model);

	modelToonRenderer_.EndPass();
}
