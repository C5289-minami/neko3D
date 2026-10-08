// デバッグ用カメラの初期化、視点操作、移動を処理する。
#include "Debug_camera.h"

#include "DxPlus/DxPlus.h"
#include "DxLib.h"
#ifndef NDEBUG
#include "imgui.h"
#endif

#include <algorithm>
#include <cmath>

namespace
{
    // マウス1ピクセルあたりの回転角度
    constexpr float c_MouseRotationRadiansPerPixel = DxPlus::PI * 2.0f / DxPlus::CLIENT_WIDTH;

    // カメラが真上・真下を向いて操作しにくくなるのを防ぐ
    constexpr float kMinPitch = DxPlus::Deg2Rad * -89.0f;
    constexpr float kMaxPitch = DxPlus::Deg2Rad * 89.0f;

    // Shiftキーを押したときの移動速度倍率
    constexpr float kFastMoveMultiplier = 3.0f;
}

void Debug_camera::Initialize(const Vec3& cameraEye, const Vec3& cameraTarget)
{
    eye = cameraEye;
    target = cameraTarget;

    // 初期視点を保存し、あとで視点をリセットできるようにする
    homeEye = cameraEye;
    homeTarget = cameraTarget;

    // 視線方向からカメラの上下角（pitch）と左右角（yaw）を求める
    const Vec3 direction = (target - eye).Normalized();
    pitch = std::asin(std::clamp(direction.y, -1.0f, 1.0f));
    yaw = std::atan2(direction.x, direction.z);

    active = false;
}

void Debug_camera::Begin()
{
    active = true;
}

void Debug_camera::SetPosition(const Vec3& position)
{
    eye = position;
    RebuildTarget();
}

void Debug_camera::ResetView()
{
    // 初期視点に戻しつつ、デバッグカメラの有効状態は維持する
    const bool wasActive = active;
    Initialize(homeEye, homeTarget);
    active = wasActive;
}

void Debug_camera::FocusAt(const Vec3& point, float distance)
{
    // yaw・pitchから視線方向を作り、指定地点を指定距離から見る
    const float cosPitch = std::cos(pitch);
    const Vec3 forward{ std::sin(yaw) * cosPitch, std::sin(pitch), std::cos(yaw) * cosPitch };
    target = point;
    eye = point - forward * distance;
    RebuildTarget();
}

void Debug_camera::Update(float deltaTime, const Vec3& focusPoint, const Vec3& viewEye, const Vec3& viewTarget)
{
#ifndef NDEBUG
    // デバッグビルド時のみ、Alt + Enterでカメラ操作を切り替える
    // 前のフレームのUI入力状態を確認する
    const bool keyboardCaptured = ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureKeyboard;
    const bool mouseCaptured = ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureMouse;
    const bool toggleDown = DxLib::CheckHitKey(KEY_INPUT_LALT) != 0 &&//左Alt
        DxLib::CheckHitKey(KEY_INPUT_RETURN) != 0;                    //Enter が押されているか
    if (toggleDown && !wasToggleDown)
    {
        if (active)
            active = false;
        else
        {
            Initialize(viewEye, viewTarget);
            Begin();
            wasToggleDown = toggleDown;
            return;
        }
    }
    wasToggleDown = toggleDown;

    // カメラ操作が無効なら、通常の視点を変更しない
    if (!active)
        return;

    // Rキーを押した瞬間に初期視点へ戻す
    const bool resetDown = !keyboardCaptured && DxLib::CheckHitKey(KEY_INPUT_R) != 0;
    if (resetDown && !wasResetDown&&active)
        ResetView();
    wasResetDown = resetDown;

    // Fキーを押した瞬間に指定地点へ注目する
    const bool focusDown = !keyboardCaptured && DxLib::CheckHitKey(KEY_INPUT_F) != 0;
    if (focusDown && !wasFocusDown)
        FocusAt(focusPoint);
    wasFocusDown = focusDown;

    // マウス移動量に応じてカメラの向きを変更する
    const DxPlus::Vec2Int mouseDelta = mouseCaptured ? DxPlus::Vec2Int{} : DxPlus::Input::GetMouseDelta();
    yaw += mouseDelta.x * c_MouseRotationRadiansPerPixel;
    pitch = std::clamp(pitch - mouseDelta.y * c_MouseRotationRadiansPerPixel, kMinPitch, kMaxPitch);

    // カメラの向きから前・右方向を求める
    const float cosPitch = std::cos(pitch);
    const Vec3 forward{ std::sin(yaw) * cosPitch, std::sin(pitch), std::cos(yaw) * cosPitch };
    const Vec3 right = Vec3::Cross(Vec3::Up(), forward).Normalized();
    Vec3 movement{};

    // キー入力から移動方向を作る
    if (!keyboardCaptured && DxLib::CheckHitKey(KEY_INPUT_W)) movement += forward;
    if (!keyboardCaptured && DxLib::CheckHitKey(KEY_INPUT_S)) movement -= forward;
    if (!keyboardCaptured && DxLib::CheckHitKey(KEY_INPUT_D)) movement += right;
    if (!keyboardCaptured && DxLib::CheckHitKey(KEY_INPUT_A)) movement -= right;
    if (!keyboardCaptured && DxLib::CheckHitKey(KEY_INPUT_E)) movement += Vec3::Up();
    if (!keyboardCaptured && DxLib::CheckHitKey(KEY_INPUT_Q)) movement -= Vec3::Up();

    if (movement.LengthSq() > 0.0f)
    {
        // 斜め移動が速くならないよう方向を正規化してから移動する
        const float speed = moveSpeed *
            (DxLib::CheckHitKey(KEY_INPUT_LSHIFT) ? kFastMoveMultiplier : 1.0f);
        eye += movement.Normalized() * (speed * deltaTime);
    }

    // カメラ位置と向きから注視点を更新する
    RebuildTarget();
#else
    (void)deltaTime;
    (void)focusPoint;
    (void)viewEye;
    (void)viewTarget;
#endif
}

void Debug_camera::RebuildTarget()
{
    // 現在のyaw・pitchに基づき、カメラの正面1単位先を注視点にする
    const float cosPitch = std::cos(pitch);
    const Vec3 forward{ std::sin(yaw) * cosPitch, std::sin(pitch), std::cos(yaw) * cosPitch };
    target = eye + forward;
}
