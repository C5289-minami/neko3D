#include "RoomTestScene.h"
#include "GameContext.h"

#ifndef NDEBUG


namespace
{
	// ImGuiでVec3を編集するためのヘルパー関数
	bool EditVector3(const char* label, Vec3& value, float speed)
	{
		float components[] = { value.x, value.y, value.z };
		if (!ImGui::DragFloat3(label, components, speed)) return false;
		value = { components[0], components[1], components[2] };
		return true;
	}
}
#endif // !NDEBUG

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
		targetCamera_.Reset();
		cameraDirector_.Reset();

		// 演出用アングルA：プレイヤーの右後ろから見上げる
		auto& player = gameContext->GetPlayer();

		Camera::CameraPose shotA{};
		shotA.fieldOfViewDegrees = 150.0f;

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

	float posY = player.GetPosition().y;
	player.Update(deltaTime * 0.5f);
	player.SetPosition({ player.GetPosition().x, posY, player.GetPosition().z });
	room.Update(deltaTime);
	stage.UpdateAlphaModel(deltaTime, gameContext->GetPlayer());


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


	{
		// プレイヤーの前方向
		const Vec3 playerForward{
			std::sin(player.GetYaw()),
			0.0f,
			std::cos(player.GetYaw())
		};

		// 仮の注視対象：プレイヤーの前方500の地点
		// ボスが用意できたら、ここをボスの位置に置き換える
		const Vec3 focusPosition =
			player.GetPosition() + playerForward * 500.0f;

		// TargetCameraの追従姿勢を更新
		targetCamera_.Update(
			player.GetPosition(),
			focusPosition,
			deltaTime);

		// FreeCameraが選択され、追従モードのときだけ手動操作
		const bool isFreeCameraActive =
			cameraDirector_.GetActiveCamera() == &camera_;

		const bool isRightClick =
			(DxLib::GetMouseInput() & MOUSE_INPUT_RIGHT) != 0;

		if (cameraDirector_.IsFollowing() &&
			isFreeCameraActive &&
			isRightClick)
		{
			SetMouseDispFlag(FALSE);
			camera_.Update(deltaTime);
		}
		else
		{
			SetMouseDispFlag(TRUE);
		}

		// カメラの遷移を更新
		cameraDirector_.Update(deltaTime);
	}

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
	if (auto* activeCamera = cameraDirector_.GetActiveCamera())
	{
		activeCamera->ApplyPose(
			cameraDirector_.GetCurrentPose());
	}	gameContext->GetGrid().Draw();
}

void RoomTestScene::DrawImGui()
{
#ifndef  NDEBUG
	ImGui::Begin("RoomTestScene Debug");


	if (ImGui::Button(u8"TargetCameraへ切り替え"))
	{
		cameraDirector_.SwitchCamera(
			targetCamera_,
			0.8f,
			Motion::Easing::EaseInOut);
	}

	if (ImGui::Button(u8"FreeCameraへ切り替え"))
	{
		cameraDirector_.SwitchCamera(
			camera_,
			0.8f,
			Motion::Easing::EaseInOut);
	}

	ImGui::Text("CameraPose");

	Camera::CameraPose pose =
		cameraDirector_.GetCurrentPose();

	bool poseChanged = false;

	if (EditVector3("Position", pose.position, 1.0f))
		poseChanged = true;

	if (ImGui::SliderFloat(
		"Pitch",
		&pose.pitch,
		-89.0f * DxPlus::Deg2Rad,
		89.0f * DxPlus::Deg2Rad))
	{
		poseChanged = true;
	}

	if (ImGui::SliderFloat(
		"Yaw",
		&pose.yaw,
		-DxPlus::PI,
		DxPlus::PI))
	{
		poseChanged = true;
	}

	if (ImGui::SliderFloat(
		"Roll",
		&pose.roll,
		-DxPlus::PI,
		DxPlus::PI))
	{
		poseChanged = true;
	}

	if (ImGui::SliderFloat(
		"FieldOfView",
		&pose.fieldOfViewDegrees,
		10.0f,
		150.0f))
	{
		poseChanged = true;
	}

	// 実際に値を編集したときだけ姿勢を更新する
	if (poseChanged)
	{
		cameraDirector_.MoveTo(
			pose,
			0.0f,
			Motion::Easing::EaseInOut);
	}




	if (ImGui::Button(u8"演出用アングルA"))
	{

		cameraDirector_.PlayShot(
			"LookUp",
			0.8f,
			Motion::Easing::EaseInOut);
	}
	if (ImGui::Button(u8"演出用アングルB"))
	{
		cameraDirector_.PlayShot(
			"CloseUp",
			0.5f,
			Motion::Easing::EaseInOut);
	}
	if (ImGui::Button(u8"FreeCameraに戻る"))
	{
		cameraDirector_.ReturnToPreviousCamera(
			0.6f,
			Motion::Easing::EaseInOut);
	}

	ImGui::End();

#endif // ! NDEBUG

}

void RoomTestScene::RenderDepth() const
{
	auto& room = gameContext->GetRoom();
	auto& player = gameContext->GetPlayer();
	auto& stage = gameContext->GetStage();

	// シーンのカメラを設定する
	if (auto* activeCamera = cameraDirector_.GetActiveCamera())
	{
		activeCamera->ApplyPose(
			cameraDirector_.GetCurrentPose());
	}
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
	if (auto* activeCamera = cameraDirector_.GetActiveCamera())
	{
		activeCamera->ApplyPose(
			cameraDirector_.GetCurrentPose());
	}
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
