// =============================
// Core/Camera/OrbitCamera.cpp
// =============================
#include "OrbitCamera.h"
#include "DxPlus/DxPlus.h"

void OrbitCamera::Reset()
{
    target = { 0.0f, 0.0f, 0.0f };
    yaw = 0.0f;
    pitch = 0.6f;
    distance = 800.0f;

    float cp = std::cos(pitch);
    float sp = std::sin(pitch);
    float cy = std::cos(yaw);
    float sy = std::sin(yaw);

    Vec3 offset
    {
        cp * sy, 
        sp, 
        cp * cy
    };

    eye = target + offset * distance;
    up = { 0.0f, 1.0f, 0.0f };
}

void OrbitCamera::BeginControl()
{
    // Raw Inputでは前回マウス座標の初期化は不要
}

void OrbitCamera::Update(float deltaTime)
{
    // Raw Inputで、このフレームのマウス移動量を取得する
    const DxPlus::Vec2Int mouseDelta = DxPlus::Input::GetMouseDelta();

    {
        const int dx = mouseDelta.x;
        const int dy = mouseDelta.y;

        DxPlus::Debug::SetFormatString(L"raw dx : %d", dx);
        DxPlus::Debug::SetFormatString(L"raw dy : %d", dy);

        yaw += dx * ROTATE_RAD_PER_PIXEL;
        pitch += dy * ROTATE_RAD_PER_PIXEL;
        pitch = std::clamp(pitch, PITCH_MIN, PITCH_MAX);

        DxPlus::Debug::SetFormatString(L"yaw:%g", yaw);
        DxPlus::Debug::SetFormatString(L"pitch:%g", pitch);
    }

    // WASD+EQで移動
    {
        float move = moveSpeed * deltaTime;    // このフレームでの移動量

        Vec3 viewForward = target - eye;
        if (viewForward.LengthSq() < 1e-8f) return;

        viewForward = viewForward.Normalized();
        Vec3 viewRight = Vec3::Cross(Vec3(0, 1, 0), viewForward).Normalized();
        Vec3 viewUp = Vec3::Cross(viewForward, viewRight).Normalized();

        float x{ 0.0f };
        float y{ 0.0f };
        float z{ 0.0f };

        using namespace DxPlus::Input;
        int button = GetButton(PLAYER1);
        if (button & BUTTON_LEFT)   x -= 1;
        if (button & BUTTON_RIGHT)  x += 1;
        if (button & BUTTON_UP)     z += 1;
        if (button & BUTTON_DOWN)   z -= 1;
        if (button & BUTTON_R1)     y += 1;
        if (button & BUTTON_L1)     y -= 1;

        Vec3 moveDir = viewRight * x + viewUp * y + viewForward * z;

        if (moveDir.LengthSq() > 0.0f)
        {
            moveDir = moveDir.Normalized();
            target += moveDir * move;
        }
    }

    // マウスホイールで注視点から視点までの距離を制御
    {
        int wheel = DxLib::GetMouseWheelRotVol();
        DxPlus::Debug::SetFormatString(L"wheel:%d", wheel);
        if (wheel)
        {
            float wheelStep = wheel / 120.0f;
            distance -= wheelStep * ZOOM_SPEED_PER_WHEEL;
            distance = std::clamp(distance, DISTANCE_MIN, DISTANCE_MAX);
        }
    }

    // yawとpitchをもとにeyeの座標を計算する
    {
        const float cp = std::cos(pitch);
        const float sp = std::sin(pitch);
        const float cy = std::cos(yaw);
        const float sy = std::sin(yaw);

        Vec3 offset
        {
            cp* sy,
            sp, 
            cp* cy
        };
        eye = target + offset * distance;
        up = { 0, 1, 0 };
    }
}

void OrbitCamera::SetFromLookAt(const Vec3& lookFrom, const Vec3& lookAt)
{
    eye = lookFrom;
    target = lookAt;

    // 注視点から視点に向かうベクトルを計算
    Vec3 dir = eye - target;

    // 距離を求める
    distance = dir.Length();

    if (distance <= 0.0001f)
    {
        return;
    }

    // 方向ベクトルを正規化
    dir = dir.Normalized();

    // pitchを求める
    pitch = std::asin(dir.y);
    pitch = std::clamp(pitch, PITCH_MIN, PITCH_MAX);

    // yawを求める
    yaw = std::atan2(dir.x, dir.z);
}
