// =============================
// Scenes/Game/GameTestScene.cpp
// =============================
#include "GameTestScene.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "Collision.h"
#ifndef NDEBUG
#include "imgui.h"
#endif
#include "ToonSettings.h"
#include "OutlineSettings.h"

void GameTestScene::Init()
{
	gameContext->Reset();
#ifdef _DEBUG
    Collision::RegisterDebugCanWatch(this, gameContext->GetPlayer(), gameContext->GetBoss());
#endif
	playerCamera_.Reset();
#ifndef NDEBUG
	activeCamera_ = nullptr;
	freeCamera_.Reset();
	wasFreeCameraToggleDown_ = false;
#endif


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



	StartFadeIn();
}

void GameTestScene::Update(float deltaTime)
{
	DxLib::SetBackgroundColor(gameContext->GetBgRed(), gameContext->GetBgGreen(), gameContext->GetBgBlue());

#ifndef NDEBUG
	const bool toggleDown = DxLib::CheckHitKey(KEY_INPUT_LALT) != 0 &&
		DxLib::CheckHitKey(KEY_INPUT_RETURN) != 0;
	if (toggleDown && !wasFreeCameraToggleDown_)
	{
		if (activeCamera_)
			activeCamera_ = nullptr;
		else
		{
			freeCamera_.Initialize(playerCamera_.GetPosition(), playerCamera_.GetTarget());
			activeCamera_ = &freeCamera_;
		}
	}
	wasFreeCameraToggleDown_ = toggleDown;

	if (DxLib::CheckHitKey(KEY_INPUT_R) != 0)
	{
		freeCamera_.ResetView();
	}
	if (DxLib::CheckHitKey(KEY_INPUT_F) != 0)
	{
		freeCamera_.FocusAt(gameContext->GetPlayer().GetPosition());
	}
	DxPlus::Debug::SetString(L" R: Transformリセット, F: FreeCameraをプレイヤーにフォーカス");



	if (activeCamera_ == &freeCamera_)
	{
		if (!freeCamera_.IsSceneViewActive() && (DxLib::GetMouseInput() & MOUSE_INPUT_RIGHT) != 0)
		{
			freeCamera_.Update(deltaTime);
		}
	}
	else
#endif
	{
		auto& player = gameContext->GetPlayer();
		auto& boss = gameContext->GetBoss();
		auto& stage = gameContext->GetStage();
		const Vec3 stageCenterPosition = stage.GetPosition();


		// player.Update(deltaTime, stage, stageCenterPosition);
		
		player.Update(deltaTime, stage.GetCollision(), stageCenterPosition);
		stage.UpdateAlphaModel(deltaTime, player);

		boss.Update(deltaTime);


		playerCamera_.Update(player.GetPosition(), boss.GetPosition(), deltaTime);

		using namespace DxPlus::Input;
		if (GetButtonDown(PLAYER1) & BUTTON_L3)
		{
			gameContext->GetEffectManager().PlayHit(RM().GetEffect(ResourceKeys::Effect_Test),
				player.GetPosition() + Vec3::Up() * 100.0f,
				player.GetYaw() + DX_PI_F);
		}
		gameContext->GetEffectManager().Update(deltaTime);
	}

#ifndef NDEBUG
	// 数値入力中はシーン切り替えのキーを無視する
	if (ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureKeyboard) return;
#endif

	using namespace DxPlus::Input;
	int buttonDown = GetButtonDown(PLAYER1);
	if (buttonDown & BUTTON_SELECT)
	{
		Scene* resultScene = SceneManager::GetInstance().GetScene(SceneID::Result);
		SetNextScene(resultScene);
		finished = true;    // フェード無しの場合は finished を true にしておく必要あり
		return;
	}
}

void GameTestScene::Render() const
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

void GameTestScene::DrawImGui()
{
#ifndef NDEBUG
    if (ImGui::Begin("Player Climbing / Wall Kick"))
    {
        auto& player = gameContext->GetPlayer();

        ImGui::TextUnformatted(u8"壁登り");
        float climbingSpeed = player.GetClimbingSpeed();
        float wallSlideSpeed = player.GetWallSlideSpeed();
        if (ImGui::DragFloat(u8"登る速度", &climbingSpeed, 1.0f, 0.0f, 5000.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp))
            player.SetClimbingSpeed(climbingSpeed);
        if (ImGui::DragFloat(u8"滑り落ちる速度", &wallSlideSpeed, 1.0f, 0.0f, 5000.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp))
            player.SetWallSlideSpeed(wallSlideSpeed);
        if (ImGui::Button(u8"壁登りの初期値に戻す"))
            player.ResetClimbingSpeeds();

        ImGui::Separator();
        ImGui::TextUnformatted(u8"壁キック");
        float wallJumpMoveSpeed = player.GetWallJumpMoveSpeed();
        float wallJumpSpeed = player.GetWallJumpSpeed();
        if (ImGui::DragFloat(u8"壁から離れる横速度", &wallJumpMoveSpeed, 10.0f, 0.0f, 5000.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp))
            player.SetWallJumpMoveSpeed(wallJumpMoveSpeed);
        if (ImGui::DragFloat(u8"上向きの初速", &wallJumpSpeed, 10.0f, 0.0f, 5000.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp))
            player.SetWallJumpSpeed(wallJumpSpeed);
        if (ImGui::Button(u8"壁キックの初期値に戻す"))
            player.ResetWallJumpSpeeds();

        ImGui::TextWrapped(u8"壁登りの速度は即時反映。壁キックは次に蹴ったときから反映します。");
        ImGui::TextWrapped(u8"設定は実行中のみ有効です。");
    }
    ImGui::End();
#endif
}

void GameTestScene::End()
{
#ifdef _DEBUG
    Collision::UnregisterDebugCanWatch(this);
#endif
}

void GameTestScene::RenderDepth() const
{
#ifndef NDEBUG
	(activeCamera_ ? activeCamera_ : &playerCamera_)->ApplyCamera();
#else
	playerCamera_.ApplyCamera();
#endif

	const auto& boss = gameContext->GetBoss();
	const auto& player = gameContext->GetPlayer();
	const auto& stage = gameContext->GetStage();
	const auto& room = gameContext->GetRoom();

	const ModelObject playerModel = player.GetModelObject().WithParentTransform(
		player.GetPosition(), player.GetRotation());
	const ModelObject bossModel = boss.GetModelObject().WithParentTransform(
		boss.GetPosition(), boss.GetRotation());
	const ModelObject stageModel = stage.GetModelObject().WithParentTransform(
		stage.GetPosition(), stage.GetRotation());

	modelToonRenderer_.DrawDepth(bossModel);
	modelToonRenderer_.DrawDepth(stageModel);
	modelToonRenderer_.DrawDepth(stage.GetAlphaModelObject());

	for (const auto& pillar : stage.GetPillarModelObjects())
	{
		modelToonRenderer_.DrawDepth(
			pillar.WithParentTransform(stage.GetPosition(), stage.GetRotation()));
	}

	for (const auto& model : stage.GetStageModelObjects())
	{
		modelToonRenderer_.DrawDepth(
			model.WithParentTransform(stage.GetPosition(), stage.GetRotation()));
	}

	modelToonRenderer_.DrawDepth(playerModel);
	modelToonRenderer_.DrawDepth(room.GetModelObject());
}

void GameTestScene::DrawScene(bool toonEnabled) const
{
#ifndef NDEBUG
	(activeCamera_ ? activeCamera_ : &playerCamera_)->ApplyCamera();
#else
	playerCamera_.ApplyCamera();
#endif

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

	const ModelObject playerModel = player.GetModelObject().WithParentTransform(
		player.GetPosition(), player.GetRotation());
	modelToonRenderer_.Draw(playerModel);
	// bossのモデルにboss自体の位置と回転を適用して描画する
	modelToonRenderer_.Draw(boss.GetModelObject().WithParentTransform(
		boss.GetPosition(), boss.GetRotation()));
	modelToonRenderer_.Draw(stage.GetModelObject().WithParentTransform(
		stage.GetPosition(), stage.GetRotation()));
	modelToonRenderer_.Draw(stage.GetAlphaModelObject());
	for (const auto& pillar : stage.GetPillarModelObjects())
	{
		modelToonRenderer_.Draw(pillar.WithParentTransform(stage.GetPosition(), stage.GetRotation()));
	}
	for (const auto& model : stage.GetStageModelObjects())
	{
		modelToonRenderer_.Draw(model.WithParentTransform(stage.GetPosition(), stage.GetRotation()));
	}
	effectManager.Draw();
	grid.Draw();

	modelToonRenderer_.Draw(room.GetModelObject());
	modelToonRenderer_.EndPass();
}

