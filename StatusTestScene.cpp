#include "StatusTestScene.h"
#include "GameContext.h"
#include "ResourceKeys.h"
#include "Collision.h"

void StatusTestScene::Init()
{
	// 背景色
	DxLib::SetBackgroundColor(gameContext->GetBgRed(), gameContext->GetBgGreen(), gameContext->GetBgBlue());

	camera_.Reset();
	freeCamera_.Reset();
	activeCamera_ = &freeCamera_;
	activeCamera_->ApplyCamera();

	// トゥーン描画に必要なシェーダーと設定バッファーを初期化
	if (!modelToonRenderer_.Init())
		DxPlus::Utils::FatalError(L"modelToonRendererの初期化失敗");
	if (!shaderEffects_.Init(DxPlus::CLIENT_WIDTH, DxPlus::CLIENT_HEIGHT))
		DxPlus::Utils::FatalError(L"shaderEffectsの初期化失敗");

	// toon描画とアウトライン描画を有効にする
	shaderEffects_.GetSettings().toonEnabled = true;
	shaderEffects_.GetSettings().outlineEnabled = true;

	testModel_.modelKey = ResourceKeys::Model_Test;

	player_.Init();
	player_.SetPosition({ 0.0f, 100.0f, 0.0f });
}

void StatusTestScene::Update(float deltaTime)
{
	// 背景色
	DxLib::SetBackgroundColor(gameContext->GetBgRed(), gameContext->GetBgGreen(), gameContext->GetBgBlue());

	float scale = 1.0f;
	testModel_.scale = { scale, scale, scale };

	// --- 右クリックを押している間のみカメラ操作 ---
	if ((DxLib::GetMouseInput() & MOUSE_INPUT_RIGHT) != 0)
	{
		SetMouseDispFlag(FALSE);
		activeCamera_->Update(deltaTime);
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

	player_.Update(deltaTime);

	Collision::UpdateModelContact(player_.GetModelObject(), testModel_);
}

void StatusTestScene::Render() const
{
	activeCamera_->ApplyCamera();

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


void StatusTestScene::RenderDepth() const
{
	// シーンのカメラを設定する
	activeCamera_->ApplyCamera();
	// Boneなしシェーダー
	modelToonRenderer_.SetVertexShader(ModelToonType::Model);

	testModel_.Draw();

	// Bone対応の頂点シェーダー
	modelToonRenderer_.SetVertexShader(ModelToonType::Bone);
	player_.Draw();

}

void StatusTestScene::DrawScene(bool toonEnabled) const
{
	// シーンのカメラを設定する
	activeCamera_->ApplyCamera();

	// つかわないもの
	grid_.Draw();

	// トゥーンシェーダーを使わない場合
	if (!toonEnabled)
	{
		SetUsePixelShader(-1);
		SetUseVertexShader(-1);

		testModel_.Draw();
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
	drawActor(testModel_, ModelToonType::Model);
	drawActor(player_.GetModelObject(), ModelToonType::Bone);


	modelToonRenderer_.EndPass();
}
