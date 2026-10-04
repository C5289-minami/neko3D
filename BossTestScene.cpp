#include "BossTestScene.h"
#include "DxConv.h"
#include "DxPlus/DxPlus.h"
#include "DxLib.h"
#include "ResourceKeys.h"
#include "ResourceManager.h"
#include "SceneManager.h"
#include "DrawableObject.h"

#include <algorithm>
#include <cmath>

namespace
{
    constexpr float MouseRotationRadiansPerPixel = DxPlus::PI * 2.0f / DxPlus::CLIENT_WIDTH;
    constexpr float MinPitch = DxPlus::Deg2Rad * -89.0f;
    constexpr float MaxPitch = DxPlus::Deg2Rad * 89.0f;
    constexpr float CameraMoveSpeed = 500.0f;
}

void BossTestScene::Init()
{
    DxLib::SetBackgroundColor(36, 42, 54);
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

    vertexShader = LoadVertexShader(L"./DevData/ShaderCompiler/ToonVS.vso");
    pixelShader = LoadPixelShader(L"./DevData/ShaderCompiler/ToonPS.pso");
	outlinePixelShader = LoadPixelShader(L"./DevData/ShaderCompiler/OutlinePS.pso");
	outlineVertexShader = LoadVertexShader(L"./DevData/ShaderCompiler/OutlineVS.vso");
	modelToonPixelShader = LoadPixelShader(L"./DevData/ShaderCompiler/ModelToonPS.pso");
	modelToonVertexShader = LoadVertexShader(L"./DevData/ShaderCompiler/SkinMesh4_DirLight_ToonVS.vso");

    outlineConstantBuffer_ = CreateShaderConstantBuffer(sizeof(float) * 4);

    float* outlineSize =
        static_cast<float*>(GetBufferShaderConstantBuffer(outlineConstantBuffer_));

    outlineSize[0] = 5.0f;                    // 横3px
    outlineSize[1] = 5.0f;                    // 縦3px
    outlineSize[2] = DxPlus::CLIENT_WIDTH;
    outlineSize[3] = DxPlus::CLIENT_HEIGHT;

    UpdateShaderConstantBuffer(outlineConstantBuffer_);

    StartFadeIn();
}

void BossTestScene::Update(float deltaTime)
{
    boss.Update(deltaTime);
	testModel_.rotation.z += 1.0f * deltaTime;

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
    if (skillKeyDown && !skillKeyWasDown_ && boss.GetCurrentState() != BossStateType::Attack)
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

    const DxPlus::Vec2Int mouseDelta = DxPlus::Input::GetMouseDelta();
    yaw += mouseDelta.x * MouseRotationRadiansPerPixel;
	//boss.GetModelObject().rotation = { yaw, yaw, yaw };
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

void BossTestScene::Render() const
{
    const float cosPitch = std::cos(pitch);
    const Vec3 forward{ std::sin(yaw) * cosPitch, std::sin(pitch), std::cos(yaw) * cosPitch };
    const Vec3 target = cameraEye + forward;
    DxLib::SetCameraPositionAndTargetAndUpVec(
        DxConv::ToVECTOR(cameraEye), DxConv::ToVECTOR(target), VGet(0.0f, 1.0f, 0.0f));

    if (isShaderEnabled_)
    {
        MV1SetUseOrigShader(TRUE);

        auto SetCulling = [](int modelHandle, int culling)
            {
                for (int i = 0; i < MV1GetMeshNum(modelHandle); i++)
                {
                    if (MV1GetMeshBackCulling(modelHandle, i) != DX_CULLING_NONE)
                    {
                        MV1SetMeshBackCulling(modelHandle, i, culling);
                    }
                }
            };

        SetShaderConstantBuffer(
            outlineConstantBuffer_,
            DX_SHADERTYPE_VERTEX,
            4
        );

        SetUsePixelShader(outlinePixelShader);
        SetUseVertexShader(outlineVertexShader);
        SetCulling(stage.GetModelHandle(), DX_CULLING_RIGHT);
        stage.Draw();
        SetCulling(RM().GetModel(testModel_.modelKey), DX_CULLING_RIGHT);
        testModel_.Draw();

        SetCulling(stage.GetModelHandle(), DX_CULLING_LEFT);
        SetCulling(RM().GetModel(testModel_.modelKey), DX_CULLING_LEFT);
		
        // 通常のToon
        SetUsePixelShader(pixelShader);
        SetUseVertexShader(vertexShader);
        stage.Draw();
        testModel_.Draw();

		// モデル専用Toon
     //   SetUsePixelShader(modelToonPixelShader);
       // SetUseVertexShader(modelToonVertexShader);

// モデル専用Toon


        SetUsePixelShader(modelToonPixelShader);
        SetUseVertexShader(modelToonVertexShader);

        boss.Draw();

    }
    else
    {
        stage.Draw();
        testModel_.Draw();
        boss.Draw();
    }


	

    if (isShaderEnabled_)
    {
        SetUsePixelShader(-1);
        SetUseVertexShader(-1);
        MV1SetUseOrigShader(FALSE);       // 描画後は標準に戻す
    }

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
