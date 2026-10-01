#include "Motion.h"

#include <algorithm>

Motion::Motion(float fromValue, float toValue, float motionDuration, Motion::Easing motionEasing)
{
    Start(fromValue, toValue, motionDuration, motionEasing);
}

void Motion::Start(float fromValue, float toValue, float motionDuration, Motion::Easing motionEasing)
{
    from = fromValue;
    to = toValue;
    duration = std::max(0.0f, motionDuration);
    easing = motionEasing;
    progress = 0.0f;
    direction = 1;
    finished = duration == 0.0f;
    if (finished)
    {
        progress = 1.0f;
    }
}

void Motion::SetDuration(float value)
{
    duration = std::max(0.0f, value);
    if (duration == 0.0f)
    {
        progress = direction > 0 ? 1.0f : 0.0f;
        finished = true;
        return;
    }

    finished = direction > 0 ? progress >= 1.0f : progress <= 0.0f;
}

void Motion::StartReverse()
{
    progress = 1.0f;
    direction = -1;
    finished = duration == 0.0f;
}

void Motion::Reset()
{
    progress = 0.0f;
    direction = 1;
    finished = false;
}

void Motion::Update(float deltaTime)
{
    if (finished || deltaTime <= 0.0f)
    {
        return;
    }

    if (duration == 0.0f)
    {
        progress = direction > 0 ? 1.0f : 0.0f;
        finished = true;
        return;
    }

    progress = std::clamp(progress + direction * deltaTime / duration, 0.0f, 1.0f);
    finished = direction > 0 ? progress >= 1.0f : progress <= 0.0f;
}

float Motion::Process(float deltaTime)
{
    Update(deltaTime);
    return GetValue();
}

void Motion::Increase(float deltaTime)
{
    direction = 1;
    finished = progress >= 1.0f;
    Update(deltaTime);
}

void Motion::Decrease(float deltaTime)
{
    direction = -1;
    finished = progress <= 0.0f;
    Update(deltaTime);
}

float Motion::GetEasedProgress() const
{
    // 今後ここを拡張して、自由にしたい。
    
    const float t = std::clamp(progress, 0.0f, 1.0f);
    switch (easing)
    {
    case Easing::EaseIn:
        return t * t * t; // 加速するイージング
    case Easing::EaseOut:
        // 減速するイージング
    {
        const float remainingProgress = 1.0f - t;
        return 1.0f - (remainingProgress * remainingProgress * remainingProgress);
    } 
    case Easing::EaseInOut:
    {
        // 緩急をつけるイージング
        constexpr float MidPoint = 0.5f;    // 前半(加速)と後半(減速)を切り替える中間点
        constexpr float ScaleFactor = 4.0f; // 中間点 (t=0.5) で出力が0.5になるよう調整する倍率
        if (t < MidPoint)
        {
            // 前半の区間を加速
            return ScaleFactor * t * t * t;
        }
        else
        {
            // 後半の区間を減速
            const float remainingProgress = 1.0f - t;
            return 1.0f - (ScaleFactor * remainingProgress * remainingProgress * remainingProgress);
        }
    }
    default:
        return t;
    }
}

float Motion::GetValue() const
{
    return from + (to - from) * GetEasedProgress();
}
