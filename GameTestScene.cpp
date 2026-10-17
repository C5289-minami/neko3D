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
#ifndef NDEBUG
#include "ModelObjectManager.h"

void GameScene::SetModelEditTargets()
{
	auto& player = gameContext->GetPlayer();
	auto& stage = gameContext->GetStage();
	auto& boss = gameContext->GetBoss();

	MObjs().AddActor("Player", player);
	MObjs().Add(player.GetModelObject(), player);
	MObjs().AddActor("Stage", stage);
	for (auto& model : stage.GetPillarModelObjects())
		MObjs().Add(model, stage);
	for (auto& model : stage.GetStageModelObjects())
		MObjs().Add(model, stage);
	MObjs().Add(stage.GetModelObject(), stage);

	MObjs().AddActor("Boss", boss);
	MObjs().Add(boss.GetModelObject(), boss);
}
#endif



void GameTest::Init()
{


    gameContext->Reset();
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

void GameTest::Update(float deltaTime)
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
	if( DxLib::CheckHitKey(KEY_INPUT_F) != 0)
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


		player.Update(deltaTime, stage, stageCenterPosition);
		stage.UpdateAlphaModel(deltaTime, player);
		boss.Update(deltaTime);
      ModelObject playerModel = player.GetModelObject().WithParentTransform(
		  player.GetPosition(), player.GetRotation());
	  const bool touching = Collision::UpdateModelContact(playerModel, boss.GetModelObject());
	  player.GetModelObject().collisionHighlighted = playerModel.collisionHighlighted;
      if (touching)
		{
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

void GameTest::Render() const
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

void GameTest::End()
{
}

void GameTest::RenderDepth() const
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

void GameTest::DrawScene(bool toonEnabled) const
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

