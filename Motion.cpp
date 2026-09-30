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
    const float t = std::clamp(progress, 0.0f, 1.0f);
    switch (easing)
    {
    case Easing::EaseIn:
        return t * t;
    case Easing::EaseOut:
        return t * (2.0f - t);
    case Easing::EaseInOut:
        return t * t * (3.0f - 2.0f * t);
    case Easing::Linear:
    default:
        return t;
    }
}

float Motion::GetValue() const
{
    return from + (to - from) * GetEasedProgress();
}
