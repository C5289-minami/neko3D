#include "BossTestScene.h"
#include "BossTestScene.h"
#include "DxConv.h"
#include "DxPlus/DxPlus.h"
#include "DxLib.h"
#include "ResourceKeys.h"
#include "ResourceManager.h"
#include "SceneManager.h"

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
    DxLib::SetGlobalAmbientLight(DxLib::GetColorF(0.35f, 0.35f, 0.35f, 1.0f));

    stage.modelKey = ResourceKeys::Model_Stage;
    boss.modelKey = ResourceKeys::Model_Paladin;
    boss.position = { 0.0f, 0.0f, 0.0f };
    boss.scale = { 1.0f, 1.0f, 1.0f };
    boss.rotation = { 0.0f, 0.0f, 0.0f };
    fontHandle = RM().GetFont(ResourceKeys::Font_Title);

    cameraEye = { 0.0f, 250.0f, -650.0f };
    const Vec3 direction = (Vec3{ 0.0f, 100.0f, 0.0f } - cameraEye).Normalized();
    pitch = std::asin(std::clamp(direction.y, -1.0f, 1.0f));
    yaw = std::atan2(direction.x, direction.z);

    StartFadeIn();
}

void BossTestScene::Update(float deltaTime)
{
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
    pitch = std::clamp(pitch + mouseDelta.y * MouseRotationRadiansPerPixel, MinPitch, MaxPitch);

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

    stage.Draw();
    boss.Draw();

    const int white = DxLib::GetColor(255, 255, 255);
#ifndef NDEBUG
    DxPlus::Text::DrawString(L"BOSS TEST  |  F1: Back to Title",
        { 24.0f, 24.0f }, white, DxPlus::Text::TextAlign::TOP_LEFT, { 1, 1 }, 0, fontHandle);
#else
    DxPlus::Text::DrawString(L"BOSS TEST",
        { 24.0f, 24.0f }, white, DxPlus::Text::TextAlign::TOP_LEFT, { 1, 1 }, 0, fontHandle);
#endif
    DxPlus::Text::DrawString(L"Mouse: Look   WASD: Move   Q/E: Down/Up   Shift: Fast",
        { 24.0f, 68.0f }, white, DxPlus::Text::TextAlign::TOP_LEFT, { 0.7f, 0.7f }, 0, fontHandle);
}
