#include "ModelAlpha.h"
#include "ResourceManager.h"

#include <algorithm>
#include <cmath>

void ModelAlpha::SetAlpha(float alpha)
{
    alpha_ = std::clamp(alpha, 0.0f, 1.0f);
}

void ModelAlpha::FadeTo(float targetAlpha, float speed, float deltaTime)
{
    targetAlpha = std::clamp(targetAlpha, 0.0f, 1.0f);
    const float step = std::max(speed, 0.0f) * std::max(deltaTime, 0.0f);
    const float difference = targetAlpha - alpha_;
    if (std::abs(difference) <= step)
    {
        SetAlpha(targetAlpha);
        return;
    }
    SetAlpha(alpha_ + (difference > 0.0f ? step : -step));
}

void ModelAlpha::Draw() const
{
    const int handle = RM().GetModel(model.modelKey);
    if (handle < 0) return;

    MV1SetOpacityRate(handle, alpha_);
    MV1SetUseZBuffer(handle, TRUE);
    // Translucent geometry tests against opaque objects without hiding them.
    MV1SetWriteZBuffer(handle, alpha_ >= 1.0f ? TRUE : FALSE);
    model.Draw();
}
