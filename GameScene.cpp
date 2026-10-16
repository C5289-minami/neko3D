// =============================
// Scenes/Game/GameScene.cpp
// =============================
#include "GameScene.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "Collision.h"
#ifndef NDEBUG
#include "imgui.h"
#endif

DebugSceneControls GameScene::GetDebugControls()
{
	return {
		"boss_test",
		{
			MakeDebugTransformTarget("boss_test/player", "Player", gameContext->GetPlayer()),
			MakeDebugTransformTarget("boss_test/boss", "Boss", gameContext->GetBoss())
		},
        DebugCameraControls{
            [] { return false; },
            [this] { return playerCamera_.GetPosition(); },
            [this](const Vec3& position) { playerCamera_.SetViewPosition(position); },
            [this] { playerCamera_.Reset(); },
            [this] { playerCamera_.FocusAt(gameContext->GetPlayer().GetPosition()); },
            [this] { return playerCamera_.GetAngleOffsetDegrees(); },
            [this](const Vec3& angles) { playerCamera_.SetAngleOffsetDegrees(angles); },
            true
        },
		[this] { return debugActorsPaused_; },
		[this](bool paused) { debugActorsPaused_ = paused; },
		transformInitialStatus_
	};
}

void GameScene::Init()
{
    //DxLib::SetBackgroundColor(32, 32, 32);

	TransformSettings::ResetToDefaults(GetDebugControls().targets, transformInitialStatus_);


    gameContext->Reset();
	playerCamera_.Reset();
#ifndef NDEBUG
	activeCamera_ = nullptr;
	freeCamera_.Reset();
	wasFreeCameraToggleDown_ = false;
#endif


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

void GameScene::Update(float deltaTime)
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
		TransformSettings::ResetToDefaults(GetDebugControls().targets, transformInitialStatus_);
	}
	if( DxLib::CheckHitKey(KEY_INPUT_F) != 0)
	{
		freeCamera_.FocusAt(gameContext->GetPlayer().GetPosition());
	}
	DxPlus::Debug::SetString(L" R: Transformリセット, F: FreeCameraをプレイヤーにフォーカス");



	if (activeCamera_ == &freeCamera_)
	{
		freeCamera_.InputUpdate(deltaTime);
	}
	else
#endif
	{
		const Vec3 stageCenterPosition{ 0.0f, 0.0f, 0.0f };
		auto& player = gameContext->GetPlayer();
		auto& boss = gameContext->GetBoss();
		auto& stage = gameContext->GetStage();


		player.Update(deltaTime, stage, stageCenterPosition);
		stage.UpdateAlphaModel(deltaTime, player);
		boss.Update(deltaTime);
		const bool touching = Collision::UpdateModelContact(player.GetModelObject(), boss.GetModelObject());
      if (touching)
		{
			player.SetPosition(player.GetModelObject().position);
			player.TakeDamage();
		}

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

void GameScene::End()
{
}

void GameScene::RenderDepth() const
{
#ifndef NDEBUG
	(activeCamera_ ? activeCamera_ : &playerCamera_)->ApplyCamera();
#else
	playerCamera_.ApplyCamera();
#endif

	const auto& boss = gameContext->GetBoss();
	const auto& player = gameContext->GetPlayer();
	const auto& stage = gameContext->GetStage();

	// Boneなしシェーダー
	modelToonRenderer_.SetVertexShader(ModelToonType::Model);

	boss.Draw();
	stage.Draw();
	stage.DrawAlphaModel();

	// Bone対応の頂点シェーダー
	modelToonRenderer_.SetVertexShader(ModelToonType::Bone);

	player.Draw();


}

void GameScene::DrawScene(bool toonEnabled) const
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

	// Shaderを使わないもの
	effectManager.Draw();
	grid.Draw();

	// トゥーンシェーダーを使わない場合
	if (!toonEnabled)
	{
		SetUsePixelShader(-1);
		SetUseVertexShader(-1);
		boss.Draw();
		stage.Draw();
		player.Draw();
		stage.DrawAlphaModel();
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
	drawActor(player.GetModelObject(), ModelToonType::Bone);
	drawActor(boss.GetModelObject(), ModelToonType::Model);
	drawActor(stage.GetModelObject(), ModelToonType::Model);
	drawActor(stage.GetAlphaModelObject(), ModelToonType::Model);
	for(const auto& pillar : stage.GetPillarModelObjects())
	{
		drawActor(pillar, ModelToonType::Model);
	}
	for(const auto& pillar : stage.GetStageModelObjects())
	{
		drawActor(pillar, ModelToonType::Model);
	}

	modelToonRenderer_.EndPass();
}

