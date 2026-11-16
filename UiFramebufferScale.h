#pragma once
#include <cmath>

struct UiFramebufferScale
{
    float x{ 1.0f };
    float y{ 1.0f };
};

// ImGui positions stay in Win32 client pixels. DxLib later stretches this
// render target to the client, so only rendering needs the inverse scale.
inline UiFramebufferScale CalculateUiFramebufferScale(
    float displayWidth, float displayHeight, float targetWidth, float targetHeight)
{
    if (!std::isfinite(displayWidth) || !std::isfinite(displayHeight) ||
        !std::isfinite(targetWidth) || !std::isfinite(targetHeight) ||
        displayWidth <= 0.0f || displayHeight <= 0.0f ||
        targetWidth <= 0.0f || targetHeight <= 0.0f)
        return {};
    const float x = targetWidth / displayWidth;
    const float y = targetHeight / displayHeight;
    return std::isfinite(x) && std::isfinite(y) ? UiFramebufferScale{ x, y } : UiFramebufferScale{};
}
