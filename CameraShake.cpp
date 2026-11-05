#include "CameraShake.h"

#include <algorithm>
#include <cmath>

namespace
{
    constexpr float TwoPi = 6.28318530718f;
}

void CameraShake::StartHorizontal(
    float duration,
    float strength,
    float frequency)
{
    Start(CameraShakeType::Horizontal, duration, strength, frequency);
}

void CameraShake::StartVertical(
    float duration,
    float strength,
    float frequency)
{
    Start(CameraShakeType::Vertical, duration, strength, frequency);
}

void CameraShake::StartMixed(
    float duration,
    float strength,
    float frequency) 
{
    Start(CameraShakeType::Mixed, duration, strength, frequency);
}

void CameraShake::Start(
    CameraShakeType type,
    float duration,
    float strength,
    float frequency)
{
    if (duration <= 0.0f || strength <= 0.0f || frequency <= 0.0f)
    {
        Stop();
        return;
    }

    type_ = type;
    elapsed_ = 0.0f;
    duration_ = duration;
    strength_ = strength;
    frequency_ = frequency;
    active_ = true;
}

void CameraShake::Stop()
{
    active_ = false;
    elapsed_ = 0.0f;
}

float CameraShake::GetWave(float time, float phase) const
{
    const float mainWave =
        std::sin(time * frequency_ * TwoPi + phase);

    const float subWave =
        std::sin(time * frequency_ * TwoPi * 1.73f + phase + 0.8f);

    // 2種類の波を混ぜて、単調な揺れを避ける
    return (mainWave + subWave * 0.35f) / 1.35f;
}

Vec3 CameraShake::Update(
    float deltaTime,
    const Camera::CameraPose& pose)
{
    if (!active_)
    {
        return {};
    }

    elapsed_ += (std::max)(deltaTime, 0.0f);

    if (elapsed_ >= duration_)
    {
        Stop();
        return {};
    }

    // 時間が経過するほど揺れを弱くする
    const float remaining = 1.0f - elapsed_ / duration_;
    const float amplitude = strength_ * remaining;

    const float horizontalWave = GetWave(elapsed_, 0.0f);
    const float verticalWave = GetWave(elapsed_, 1.7f);

    float horizontal = 0.0f;
    float vertical = 0.0f;

    switch (type_)
    {
    case CameraShakeType::Horizontal:
        horizontal = horizontalWave * amplitude;
        break;

    case CameraShakeType::Vertical:
        vertical = verticalWave * amplitude;
        break;

    case CameraShakeType::Mixed:
        horizontal = horizontalWave * amplitude;
        vertical = verticalWave * amplitude;
        break;
    }

    // カメラの向きに応じた画面の右方向
    const Vec3 right{
        std::cos(pose.yaw),
        0.0f,
        -std::sin(pose.yaw)
    };

    // カメラの向きに応じた画面の上方向
    const Vec3 up{
        -std::sin(pose.yaw) * std::sin(pose.pitch),
        std::cos(pose.pitch),
        -std::cos(pose.yaw) * std::sin(pose.pitch)
    };

    return right * horizontal + up * vertical;
}