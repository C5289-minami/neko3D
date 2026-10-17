#include "RoomTestScene.h"
#include "GameContext.h"

void RoomTestScene::Init()
{
	DxLib::SetBackgroundColor(gameContext->GetBgRed(), gameContext->GetBgGreen(), gameContext->GetBgBlue());
 gameContext->GetPlayer().Init();
	room_.Init();
  gameContext->GetStage().Init();
	camera_.Initialize({ 0.0f, 400.0f, -400.0f }, { 0.0f, 100.0f, 0.0f });


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

void RoomTestScene::Update(float deltaTime)
{
	DxLib::SetBackgroundColor(gameContext->GetBgRed(), gameContext->GetBgGreen(), gameContext->GetBgBlue());
  gameContext->GetPlayer().Update(0);
	room_.Update(deltaTime);
    gameContext->GetStage().UpdateAlphaModel(deltaTime, gameContext->GetPlayer());

	// --- 右クリックを押している間のみカメラ操作 ---
	if ((DxLib::GetMouseInput() & MOUSE_INPUT_RIGHT) != 0)
	{
		SetMouseDispFlag(FALSE);
		camera_.Update(deltaTime);
	}
	else SetMouseDispFlag(TRUE);

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

void RoomTestScene::Render() const
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
	camera_.ApplyCamera();
   gameContext->GetGrid().Draw();
}

void RoomTestScene::RenderDepth() const
{
	// シーンのカメラを設定する
	camera_.ApplyCamera();
	// Boneなしシェーダー
	modelToonRenderer_.SetVertexShader(ModelToonType::Model);

	room_.Draw();
  gameContext->GetStage().Draw();

	// Bone対応の頂点シェーダー
	modelToonRenderer_.SetVertexShader(ModelToonType::Bone);

 gameContext->GetPlayer().Draw();

}

void RoomTestScene::DrawScene(bool toonEnabled) const
{
	// カメラを設定する
	camera_.ApplyCamera();
	// トゥーンシェーダーを使わない場合
	if (!toonEnabled)
	{
		SetUsePixelShader(-1);
		SetUseVertexShader(-1);

     gameContext->GetPlayer().Draw();
		room_.Draw();

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
   const auto& player = gameContext->GetPlayer();
   const ModelObject playerModel = player.GetModelObject().WithParentTransform(
	   player.GetPosition(), player.GetRotation());
   drawActor(playerModel, ModelToonType::Bone);
	drawActor(room_.GetModelObject(), ModelToonType::Model);
   drawActor(gameContext->GetStage().GetModelObject(), ModelToonType::Model);
	for (const auto& pillar : gameContext->GetStage().GetPillarModelObjects())
	{
		drawActor(pillar, ModelToonType::Model);
	}
 for (const auto& model : gameContext->GetStage().GetStageModelObjects())
	{
		drawActor(model, ModelToonType::Model);
	}

	modelToonRenderer_.EndPass();
}
