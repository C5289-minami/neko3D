#include "BossTestScene.h"
#include "DxConv.h"
#include "DxPlus/DxPlus.h"
#include "DxLib.h"
#include "ResourceKeys.h"
#include "ResourceManager.h"
#include "SceneManager.h"
#include "DrawableObject.h"
#include "ToonSettings.h"
#include "ModelToonRenderer.h"

#include <algorithm>
#include <cmath>
#ifndef NDEBUG
#include "imgui.h"
#endif

#include "Player.h"
#include "Sound3D.h"
#include "OutlineSettings.h"

namespace
{
	constexpr float MouseRotationRadiansPerPixel =
		DxPlus::PI * 2.0f / DxPlus::CLIENT_WIDTH;

	constexpr float MinPitch = DxPlus::Deg2Rad * -89.0f;
	constexpr float MaxPitch = DxPlus::Deg2Rad * 89.0f;
	constexpr float CameraMoveSpeed = 500.0f;

	constexpr int ToonSettingsSlot = 5;
	constexpr int OutlineConstantBufferSlot = 4;

	void SetModelCulling(int modelHandle, int culling)
	{
		if (modelHandle < 0)
		{
			return;
		}

		for (int i = 0; i < MV1GetMeshNum(modelHandle); ++i)
		{
			if (MV1GetMeshBackCulling(modelHandle, i) != DX_CULLING_NONE)
			{
				MV1SetMeshBackCulling(modelHandle, i, culling);
			}
		}
	}
}

DebugSceneControls BossTestScene::GetDebugControls()
{
    return {
        "boss_test",
        {
            MakeDebugTransformTarget("boss_test/player", "Player", test),
            MakeDebugTransformTarget("boss_test/boss", "Boss", boss)
        },
        DebugCameraControls{
            [this] { return debugCamera_.IsSceneViewActive(); },
            [this] { return debugCamera_.IsSceneViewActive() ? debugCamera_.GetEye() : cameraEye; },
            [this](const Vec3& position) {
                if (debugCamera_.IsSceneViewActive()) debugCamera_.SetPosition(position);
                else cameraEye = position;
            },
            [this] {
                if (debugCamera_.IsSceneViewActive()) debugCamera_.ResetView();
                else cameraEye = { 0.0f, 250.0f, -650.0f };
            },
            [this] {
                if (!debugCamera_.IsSceneViewActive())
                {
                    const float cosPitch = std::cos(pitch);
                    const Vec3 forward{ std::sin(yaw) * cosPitch, std::sin(pitch), std::cos(yaw) * cosPitch };
                    debugCamera_.Initialize(cameraEye, cameraEye + forward);
                    debugCamera_.Begin();
                }
                debugCamera_.FocusAt(test.GetPosition());
            }
        },
        [this] { return debugActorsPaused_; },
        [this](bool paused) { debugActorsPaused_ = paused; },
        transformInitialStatus_
    };
}

void BossTestScene::Init()
{
	DxLib::SetBackgroundColor(255, 178, 102);
	SetBackgroundColor(40, 40, 40);
	DxLib::SetLightDirection(VGet(-0.3f, -1.0f, -0.5f));
	// DxLib::SetGlobalAmbientLight(DxLib::GetColorF(0.35f, 0.35f, 0.35f, 1.0f));


	stage.Init();
	stage.Reset();
	MV1SetMeshCategoryVisible(
		stage.GetModelHandle(),
		DX_MV1_MESHCATEGORY_OUTLINE_ORIG_SHADER,
		TRUE
	);

	boss.Init();

	boss.Reset();


	fontHandle = RM().GetFont(ResourceKeys::Font_Title);

	cameraEye = { 0.0f, 250.0f, -650.0f };
	const Vec3 direction = (Vec3{ 0.0f, 100.0f, 0.0f } - cameraEye).Normalized();
	pitch = std::asin(std::clamp(direction.y, -1.0f, 1.0f));
	yaw = std::atan2(direction.x, direction.z);

	testModel_.modelKey = ResourceKeys::Model_Test;
	testModel_.scale = { 500.0f, 500.0f, 500.0f };

	vertexShader = LoadVertexShader(L"./DevData/ShaderCompiler/Bin/ToonVS.vso");
	pixelShader = LoadPixelShader(L"./DevData/ShaderCompiler/Bin/ToonPS.pso");
	outlinePixelShader = LoadPixelShader(L"./DevData/ShaderCompiler/Bin/OutlinePS.pso");
	outlineVertexShader = LoadVertexShader(L"./DevData/ShaderCompiler/Bin/OutlineVS.vso");
	modelToonRenderer_.Init();
	outlineConstantBuffer_ = CreateShaderConstantBuffer(sizeof(float) * 4);

	float* outlineSize =
		static_cast<float*>(GetBufferShaderConstantBuffer(outlineConstantBuffer_));

	toonConstantBuffer_ = CreateShaderConstantBuffer(sizeof(ToonSettings));

	ToonSettings* settings =
		static_cast<ToonSettings*>(
			GetBufferShaderConstantBuffer(toonConstantBuffer_)
			);

	*settings = g_toonSettings;

	outlineSize[0] = 5.0f;                    // 横3px
	outlineSize[1] = 5.0f;                    // 縦3px
	outlineSize[2] = DxPlus::CLIENT_WIDTH;
	outlineSize[3] = DxPlus::CLIENT_HEIGHT;

	UpdateShaderConstantBuffer(toonConstantBuffer_);

	UpdateShaderConstantBuffer(outlineConstantBuffer_);

	depthPixelShader_ =
		LoadPixelShader(
			L"./DevData/ShaderCompiler/Bin/DepthPS.pso"
		);
	SetDrawValidFloatTypeGraphCreateFlag(TRUE);
	SetCreateDrawValidGraphChannelNum(4);
	SetCreateGraphChannelBitDepth(32);

	depthBuffer_ =
		MakeScreen(
			DxPlus::CLIENT_WIDTH,
			DxPlus::CLIENT_HEIGHT,
			TRUE
		);

	depthViewPixelShader_ =
		LoadPixelShader(
			L"./DevData/ShaderCompiler/Bin/DepthViewPS.pso"
		);
	{
		const float width =
			static_cast<float>(DxPlus::CLIENT_WIDTH);

		const float height =
			static_cast<float>(DxPlus::CLIENT_HEIGHT);

		depthViewVertices_[0].pos = VGet(0.0f, 0.0f, 0.0f);
		depthViewVertices_[1].pos = VGet(width, 0.0f, 0.0f);
		depthViewVertices_[2].pos = VGet(0.0f, height, 0.0f);
		depthViewVertices_[3].pos = VGet(width, height, 0.0f);
		depthViewVertices_[4] = depthViewVertices_[2];
		depthViewVertices_[5] = depthViewVertices_[1];

		for (int i = 0; i < 6; ++i)
		{
			depthViewVertices_[i].rhw = 1.0f;
			depthViewVertices_[i].dif =
				GetColorU8(255, 255, 255, 255);
			depthViewVertices_[i].spc =
				GetColorU8(0, 0, 0, 0);
		}

		depthViewVertices_[0].u = 0.0f;
		depthViewVertices_[0].v = 0.0f;

		depthViewVertices_[1].u = 1.0f;
		depthViewVertices_[1].v = 0.0f;

		depthViewVertices_[2].u = 0.0f;
		depthViewVertices_[2].v = 1.0f;

		depthViewVertices_[3].u = 1.0f;
		depthViewVertices_[3].v = 1.0f;

		depthViewVertices_[4] = depthViewVertices_[2];
		depthViewVertices_[5] = depthViewVertices_[1];

		depthViewVertices_[0].su = 0.0f;
		depthViewVertices_[0].sv = 0.0f;
		depthViewVertices_[1].su = 1.0f;
		depthViewVertices_[1].sv = 0.0f;
		depthViewVertices_[2].su = 0.0f;
		depthViewVertices_[2].sv = 1.0f;
		depthViewVertices_[3].su = 1.0f;
		depthViewVertices_[3].sv = 1.0f;
		depthViewVertices_[4].su = 0.0f;
		depthViewVertices_[4].sv = 1.0f;
		depthViewVertices_[5].su = 1.0f;
		depthViewVertices_[5].sv = 0.0f;

	}
	sceneBuffer_ =
		MakeScreen(
			DxPlus::CLIENT_WIDTH,
			DxPlus::CLIENT_HEIGHT,
			TRUE
		);

	SetUseGraphZBuffer(sceneBuffer_, TRUE);
	postProcessPixelShader_ =
		LoadPixelShader(
			L"./DevData/ShaderCompiler/Bin/PostProcessPS.pso"
		);

	outlineSettingsConstantBuffer_ =
		CreateShaderConstantBuffer(sizeof(OutlineSettings));

	OutlineSettings* outlineSettings =
		static_cast<OutlineSettings*>(
			GetBufferShaderConstantBuffer(
				outlineSettingsConstantBuffer_
			)
			);

	*outlineSettings = g_outlineSettings;

	UpdateShaderConstantBuffer(
		outlineSettingsConstantBuffer_
	);

	// 作成設定を元に戻す
	SetDrawValidFloatTypeGraphCreateFlag(FALSE);
	SetCreateDrawValidGraphChannelNum(0);
	SetCreateGraphChannelBitDepth(0);

	// この画面専用のZバッファを作る
	SetUseGraphZBuffer(depthBuffer_, TRUE);

	StartFadeIn();

    // プレイヤーの初期設定
	test.Init();
	test.Reset();
   test.Update(0, stage);
   debugActorsPaused_ = false;
   TransformSettings::ResetToDefaults(GetDebugControls().targets, transformInitialStatus_);
   const float cosPitch = std::cos(pitch);
   const Vec3 forward{ std::sin(yaw) * cosPitch, std::sin(pitch), std::cos(yaw) * cosPitch };
   debugCamera_.Initialize(cameraEye, cameraEye + forward);

}

void BossTestScene::Update(float deltaTime)
{

#ifndef NDEBUG
    const float cosPitch = std::cos(pitch);
    const Vec3 forward{ std::sin(yaw) * cosPitch, std::sin(pitch), std::cos(yaw) * cosPitch };
    debugCamera_.Update(deltaTime, test.GetPosition(), cameraEye, cameraEye + forward);
    const bool actorsPaused = debugActorsPaused_ || debugCamera_.IsSceneViewActive();
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
    if (!actorsPaused) boss.Update(deltaTime);
	if (!actorsPaused) testModel_.rotation.z += 1.0f * deltaTime;

    // --- シェーダーON/OFF切り替え (F2キー) ---
    const bool shaderKeyDown = DxLib::CheckHitKey(KEY_INPUT_F2) != 0;
    if (shaderKeyDown && !shaderKeyWasDown_)
    {
        isShaderEnabled_ = !isShaderEnabled_; // フラグを反転
    }
    shaderKeyWasDown_ = shaderKeyDown;

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
    if (!mouseCaptured && !debugCamera_.IsSceneViewActive() && (DxLib::GetMouseInput() & MOUSE_INPUT_RIGHT) != 0)
    {
        const DxPlus::Vec2Int mouseDelta = DxPlus::Input::GetMouseDelta();
        yaw += mouseDelta.x * MouseRotationRadiansPerPixel;
        pitch = std::clamp(pitch - mouseDelta.y * MouseRotationRadiansPerPixel, MinPitch, MaxPitch);

        const float cosPitch = std::cos(pitch);
        const Vec3 forward{ std::sin(yaw) * cosPitch, std::sin(pitch), std::cos(yaw) * cosPitch };
        const Vec3 right = Vec3::Cross(Vec3::Up(), forward).Normalized();
        Vec3 movement{};

        if (DxLib::CheckHitKey(KEY_INPUT_W)) movement += forward;
        if (DxLib::CheckHitKey(KEY_INPUT_S)) movement -= forward;
        if (DxLib::CheckHitKey(KEY_INPUT_D)) movement += right;
        if (DxLib::CheckHitKey(KEY_INPUT_A)) movement -= right;
        if (DxLib::CheckHitKey(KEY_INPUT_E)) movement += Vec3::Up();
        if (DxLib::CheckHitKey(KEY_INPUT_Q)) movement -= Vec3::Up();

        if (movement.LengthSq() > 0.0f)
        {
            const float speed = CameraMoveSpeed * (DxLib::CheckHitKey(KEY_INPUT_LSHIFT) ? 3.0f : 1.0f);
            cameraEye += movement.Normalized() * (speed * deltaTime);
        }
    }
	//test.Update(deltaTime, stage);



	Sound3D::SetListener(
		cameraEye,
		Vec3{ std::sin(yaw) * std::cos(pitch), std::sin(pitch), std::cos(yaw) * std::cos(pitch) }
	);
	Sound3D::Play(RM().GetSound(ResourceKeys::Sound_BossBite), boss.GetPosition());
}

void BossTestScene::Render() const
{
	const float cosPitch = std::cos(pitch);
	const Vec3 forward{
		std::sin(yaw) * cosPitch,
		std::sin(pitch),
		std::cos(yaw) * cosPitch
	};

	const Vec3 target = cameraEye + forward;

	DxLib::SetCameraPositionAndTargetAndUpVec(
		DxConv::ToVECTOR(cameraEye),
		DxConv::ToVECTOR(target),
		VGet(0.0f, 1.0f, 0.0f)
	);


	RenderSceneBuffer();

	RenderDepth();

	RenderPostProcess();

	//DrawExtendGraph(
	//    0,
	//    0,
	//    DxPlus::CLIENT_WIDTH,
	//    DxPlus::CLIENT_HEIGHT,
	//    depthBuffer_,
	//    FALSE
	//);

  //  if (isShaderEnabled_)
  //  {
  //      MV1SetUseOrigShader(TRUE);

  //      ToonSettings* settings =
  //          static_cast<ToonSettings*>(
  //              GetBufferShaderConstantBuffer(toonConstantBuffer_)
  //              );

  //      *settings = g_toonSettings;

  //      UpdateShaderConstantBuffer(toonConstantBuffer_);

  //      SetShaderConstantBuffer(
  //          toonConstantBuffer_,
  //          DX_SHADERTYPE_PIXEL,
  //          ToonSettingsSlot
  //      );

  //      SetShaderConstantBuffer(
  //          outlineConstantBuffer_,
  //          DX_SHADERTYPE_VERTEX,
  //          OutlineConstantBufferSlot
  //      );

  //      const int stageModelHandle = stage.GetModelHandle();
  //      const int testModelHandle = RM().GetModel(testModel_.modelKey);

  //      // ========================================
  //      // Outline
  //      // ========================================

  //      SetUsePixelShader(outlinePixelShader);
  //      SetUseVertexShader(outlineVertexShader);

  //      SetModelCulling(stageModelHandle, DX_CULLING_RIGHT);
  //      SetModelCulling(testModelHandle, DX_CULLING_RIGHT);

  //      stage.Draw();
  //      testModel_.Draw();

  //      // ========================================
  //      // 通常Toon
  //      // ========================================

  //      SetModelCulling(stageModelHandle, DX_CULLING_LEFT);
  //      SetModelCulling(testModelHandle, DX_CULLING_LEFT);

  //      SetUsePixelShader(pixelShader);
  //      SetUseVertexShader(vertexShader);

  //      stage.Draw();
  //      testModel_.Draw();

  //      modelToonRenderer_.Draw(
  //          test.GetModelObject(),
  //          ModelToonType::NMap4Frame
  //      );
		//test.Draw();

  //      modelToonRenderer_.Draw(
  //          boss.GetModelObject(),
  //          ModelToonType::FourFrame
  //      );
  //  }
  //  else
  //  {
  //      stage.Draw();
  //      testModel_.Draw();
  //      boss.Draw();
  //      test.Draw();
  //  }

  //  if (isShaderEnabled_)
  //  {
  //      SetUsePixelShader(-1);
  //      SetUseVertexShader(-1);
  //      MV1SetUseOrigShader(FALSE);       // 描画後は標準に戻す
  //  }


	const int white = DxLib::GetColor(255, 255, 255);
	const int green = DxLib::GetColor(100, 255, 100);
	const int red = DxLib::GetColor(255, 100, 100);

#ifndef NDEBUG
	DxPlus::Text::DrawString(L"BOSS TEST  |  F1: Back to Title",
		{ 24.0f, 24.0f }, white, DxPlus::Text::TextAlign::TOP_LEFT, { 1, 1 }, 0, fontHandle);
#else
	DxPlus::Text::DrawString(L"BOSS TEST",
		{ 24.0f, 24.0f }, white, DxPlus::Text::TextAlign::TOP_LEFT, { 1, 1 }, 0, fontHandle);
#endif

	// 現在のシェーダー状態を表示 (ONなら緑、OFFなら赤)
	const std::wstring shaderStatus = L"Shader [F2]: " + std::wstring(isShaderEnabled_ ? L"ON" : L"OFF");
	DxPlus::Text::DrawString(shaderStatus.c_str(),
		{ 24.0f, 68.0f }, isShaderEnabled_ ? green : red, DxPlus::Text::TextAlign::TOP_LEFT, { 0.8f, 0.8f }, 0, fontHandle);

	DxPlus::Text::DrawString(L"1: Bang   2: Tail Whip   3: Clap   4: HairBall   5: Bite",
		{ 24.0f, 100.0f }, white, DxPlus::Text::TextAlign::TOP_LEFT, { 0.7f, 0.7f }, 0, fontHandle);
	DxPlus::Text::DrawString(L"Mouse: Look   WASD: Move   Q/E: Down/Up   Shift: Fast",
		{ 24.0f, 132.0f }, white, DxPlus::Text::TextAlign::TOP_LEFT, { 0.7f, 0.7f }, 0, fontHandle);
}

void BossTestScene::RenderDepth() const
{
	MV1SetUseOrigShader(TRUE);

	SetDrawScreen(depthBuffer_);
	SetDrawZBuffer(depthBuffer_);

	const float cosPitch = std::cos(pitch);

	const Vec3 forward{
		std::sin(yaw) * cosPitch,
		std::sin(pitch),
		std::cos(yaw) * cosPitch
	};

	const Vec3 target =
		cameraEye + forward;

	DxLib::SetCameraPositionAndTargetAndUpVec(
		DxConv::ToVECTOR(cameraEye),
		DxConv::ToVECTOR(target),
		VGet(0.0f, 1.0f, 0.0f)
	);

	// 深度テストを有効にする
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);

	ClearDrawScreen();
	ClearDrawScreenZBuffer();

	// 深度＋法線を書き込む
	SetUsePixelShader(depthPixelShader_);

	// --------------------------------
	// Stage / OBJ
	// --------------------------------

	SetUseVertexShader(vertexShader);

	//stage.Draw();
	//testModel_.Draw();

	// --------------------------------
	// 旧Boss
	// --------------------------------

	modelToonRenderer_.SetVertexShader(
		ModelToonType::NMap4Frame
	);

	test.Draw();

	// --------------------------------
	// 新Boss
	// --------------------------------

	modelToonRenderer_.SetVertexShader(
		ModelToonType::FourFrame
	);

	boss.Draw();

	// --------------------------------
	// 後片付け
	// --------------------------------

	SetUsePixelShader(-1);
	SetUseVertexShader(-1);

	SetUseZBuffer3D(FALSE);
	SetWriteZBuffer3D(FALSE);

	SetDrawZBuffer(-1);
	SetDrawScreen(DX_SCREEN_BACK);

	MV1SetUseOrigShader(FALSE);
}

void BossTestScene::RenderDepthView() const
{
	SetUseTextureToShader(
		0,
		depthBuffer_
	);

	SetUsePixelShader(
		depthViewPixelShader_
	);

	DrawPrimitive2DToShader(
		depthViewVertices_,
		6,
		DX_PRIMTYPE_TRIANGLELIST
	);

	SetUsePixelShader(-1);

	SetUseTextureToShader(
		0,
		-1
	);
}

void BossTestScene::RenderSceneBuffer() const
{
	SetDrawScreen(sceneBuffer_);
	SetDrawZBuffer(sceneBuffer_);

	const float cosPitch = std::cos(pitch);

	const Vec3 forward{
		std::sin(yaw) * cosPitch,
		std::sin(pitch),
		std::cos(yaw) * cosPitch
	};

	const Vec3 target =
		cameraEye + forward;

	DxLib::SetCameraPositionAndTargetAndUpVec(
		DxConv::ToVECTOR(cameraEye),
		DxConv::ToVECTOR(target),
		VGet(0.0f, 1.0f, 0.0f)
	);

	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);

	ClearDrawScreen();
	ClearDrawScreenZBuffer();

	MV1SetUseOrigShader(TRUE);

	ToonSettings* settings =
		static_cast<ToonSettings*>(
			GetBufferShaderConstantBuffer(toonConstantBuffer_)
			);

	*settings = g_toonSettings;

	UpdateShaderConstantBuffer(toonConstantBuffer_);

	SetShaderConstantBuffer(
		toonConstantBuffer_,
		DX_SHADERTYPE_PIXEL,
		ToonSettingsSlot
	);

	SetShaderConstantBuffer(
		outlineConstantBuffer_,
		DX_SHADERTYPE_VERTEX,
		OutlineConstantBufferSlot
	);

	const int stageModelHandle =
		stage.GetModelHandle();

	const int testModelHandle =
		RM().GetModel(testModel_.modelKey);


	// -----------------------------
	// Toon
	// -----------------------------

	SetModelCulling(
		stageModelHandle,
		DX_CULLING_LEFT
	);

	SetModelCulling(
		testModelHandle,
		DX_CULLING_LEFT
	);

	SetUsePixelShader(pixelShader);
	SetUseVertexShader(vertexShader);

	stage.Draw();
	testModel_.Draw();

	// -----------------------------
	// 旧Boss
	// -----------------------------

	modelToonRenderer_.Draw(
		test.GetModelObject(),
		ModelToonType::NMap4Frame
	);

	// -----------------------------
	// 新Boss
	// -----------------------------

	modelToonRenderer_.Draw(
		boss.GetModelObject(),
		ModelToonType::FourFrame
	);

	SetUsePixelShader(-1);
	SetUseVertexShader(-1);

	MV1SetUseOrigShader(FALSE);

	SetUseZBuffer3D(FALSE);
	SetWriteZBuffer3D(FALSE);

	SetDrawZBuffer(-1);
	SetDrawScreen(DX_SCREEN_BACK);
}
void BossTestScene::RenderPostProcess() const
{
	OutlineSettings* outlineSettings =
		static_cast<OutlineSettings*>(
			GetBufferShaderConstantBuffer(
				outlineSettingsConstantBuffer_
			)
			);

	*outlineSettings = g_outlineSettings;

	UpdateShaderConstantBuffer(
		outlineSettingsConstantBuffer_
	);

	SetShaderConstantBuffer(
		outlineSettingsConstantBuffer_,
		DX_SHADERTYPE_PIXEL,
		6
	);

	SetUseTextureToShader(0, sceneBuffer_);
	SetUseTextureToShader(1, depthBuffer_);

	SetUsePixelShader(postProcessPixelShader_);

	DrawPrimitive2DToShader(
		depthViewVertices_,
		6,
		DX_PRIMTYPE_TRIANGLELIST
	);

	SetUsePixelShader(-1);

	SetUseTextureToShader(0, -1);
	SetUseTextureToShader(1, -1);
}