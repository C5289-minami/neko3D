#include "RoomTestScene.h"
#include "GameContext.h"

void RoomTestScene::Init()
{
	DxLib::SetBackgroundColor(
		gameContext->GetBgRed(),
		gameContext->GetBgGreen(),
		gameContext->GetBgBlue());

	{
		camera_.Initialize(
			{ 0.0f, 400.0f, -400.0f },
			{ 0.0f, 100.0f, 0.0f });

		// カメラ初期化後にDirectorの姿勢を同期
		cameraDirector_.Reset();

		// 演出用アングルA：右後ろから見上げる
		Camera::CameraPose shotA{};
		shotA.position = { 300.0f, 600.0f, -400.0f };
		shotA.pitch = -0.35f;
		shotA.yaw = 0.7f;
		shotA.roll = 0.0f;
		shotA.fieldOfViewDegrees = 50.0f;

		// 演出用アングルB：近距離の別アングル
		Camera::CameraPose shotB{};
		shotB.position = { -150.0f, 250.0f, -100.0f };
		shotB.pitch = -0.1f;
		shotB.yaw = -0.8f;
		shotB.roll = 0.0f;
		shotB.fieldOfViewDegrees = 40.0f;

		// 登録
		cameraDirector_.RegisterShot("LookUp", shotA);
		cameraDirector_.RegisterShot("CloseUp", shotB);
	}

	{
		// トゥーン描画に必要なシェーダーと設定バッファーを初期化
		if (!modelToonRenderer_.Init())
			DxPlus::Utils::FatalError(
				L"modelToonRendererの初期化失敗");

		if (!shaderEffects_.Init(
			DxPlus::CLIENT_WIDTH,
			DxPlus::CLIENT_HEIGHT))
		{
			DxPlus::Utils::FatalError(
				L"shaderEffectsの初期化失敗");
		}

		// toon描画とアウトライン描画を有効にする
		shaderEffects_.GetSettings().toonEnabled = true;
		shaderEffects_.GetSettings().outlineEnabled = true;
	}

	StartFadeIn();
}
void RoomTestScene::Update(float deltaTime)
{
	DxLib::SetBackgroundColor(gameContext->GetBgRed(), gameContext->GetBgGreen(), gameContext->GetBgBlue());

	auto& room = gameContext->GetRoom();
	auto& player = gameContext->GetPlayer();
	auto& stage = gameContext->GetStage();

	player.Update(0);
	room.Update(deltaTime);
	stage.UpdateAlphaModel(deltaTime, gameContext->GetPlayer());

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


	// camera

	// --- 右クリックを押している間のみカメラ操作 ---
	if ((DxLib::GetMouseInput() & MOUSE_INPUT_RIGHT) != 0)
	{
		SetMouseDispFlag(FALSE);
		camera_.Update(deltaTime);
	}
	else
	{
		SetMouseDispFlag(TRUE);
	}

	// --- カメラ遷移・姿勢の保持・最終適用 ---

	// 通常のFreeCamera使用中のみ手動操作を許可
	const bool isRightClick =
		(DxLib::GetMouseInput() & MOUSE_INPUT_RIGHT) != 0;

	if (cameraDirector_.IsFollowing() && isRightClick)
	{
		SetMouseDispFlag(FALSE);
		camera_.Update(deltaTime);
	}
	else
	{
		SetMouseDispFlag(TRUE);
	}

	// --- F3：演出用アングルA ---
	const bool cameraMoveKeyDown =
		DxLib::CheckHitKey(KEY_INPUT_F3) != 0;

	if (cameraMoveKeyDown && !wasCameraMoveKeyDown_)
	{
		cameraDirector_.PlayShot(
			"LookUp",
			0.8f,
			Motion::Easing::EaseInOut);
	}

	wasCameraMoveKeyDown_ = cameraMoveKeyDown;

	// --- F5：演出用アングルB ---
	const bool cameraCloseUpKeyDown =
		DxLib::CheckHitKey(KEY_INPUT_F5) != 0;

	if (cameraCloseUpKeyDown && !wasCameraCloseUpKeyDown_)
	{
		cameraDirector_.PlayShot(
			"CloseUp",
			0.5f,
			Motion::Easing::EaseInOut);
	}

	wasCameraCloseUpKeyDown_ = cameraCloseUpKeyDown;

	// --- F4：通常のFreeCameraの姿勢へ戻る ---
	const bool cameraFollowKeyDown =
		DxLib::CheckHitKey(KEY_INPUT_F4) != 0;

	if (cameraFollowKeyDown && !wasCameraFollowKeyDown_)
	{
		cameraDirector_.ReturnToPreviousCamera(
			0.6f,
			Motion::Easing::EaseInOut);
	}

	wasCameraFollowKeyDown_ = cameraFollowKeyDown;

	// --- カメラの遷移と最終適用 ---
	cameraDirector_.Update(deltaTime);

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
	camera_.ApplyPose(cameraDirector_.GetCurrentPose());
	gameContext->GetGrid().Draw();
}

void RoomTestScene::RenderDepth() const
{
	auto& room = gameContext->GetRoom();
	auto& player = gameContext->GetPlayer();
	auto& stage = gameContext->GetStage();

	// シーンのカメラを設定する
	camera_.ApplyPose(cameraDirector_.GetCurrentPose());
	// Boneなしシェーダー
	modelToonRenderer_.SetVertexShader(ModelType::Model);
	room.Draw();
	gameContext->GetStage().Draw();

	// Bone対応の頂点シェーダー
	modelToonRenderer_.SetVertexShader(ModelType::Bone);

	gameContext->GetPlayer().Draw();

}

void RoomTestScene::DrawScene(bool toonEnabled) const
{
	auto& room = gameContext->GetRoom();
	// カメラを設定する
	camera_.ApplyPose(cameraDirector_.GetCurrentPose());
	// トゥーンシェーダーを使わない場合
	if (!toonEnabled)
	{
		SetUsePixelShader(-1);
		SetUseVertexShader(-1);

		gameContext->GetPlayer().Draw();
		gameContext->GetStage().Draw();
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
	const auto& player = gameContext->GetPlayer();
	const ModelObject playerModel = player.GetModelObject().WithParentTransform(
		player.GetPosition(), player.GetRotation());
	drawActor(playerModel, ModelType::Bone);
	drawActor(room.GetModelObject(), ModelType::Model);
	drawActor(gameContext->GetStage().GetModelObject(), ModelType::Model);
	for (const auto& pillar : gameContext->GetStage().GetPillarModelObjects())
	{
		drawActor(pillar, ModelType::Model);
	}
	for (const auto& model : gameContext->GetStage().GetStageModelObjects())
	{
		drawActor(model, ModelType::Model);
	}

	modelToonRenderer_.EndPass();
}
