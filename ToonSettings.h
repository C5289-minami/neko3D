#pragma once

// シェーダーのトゥーン設定用


struct ToonSettings
{
    float shadowColor[4] = { 0.2f, 0.25f, 0.6f, 1.0f };

    float thresholds[4] = { 0.9f, 0.7f, 0.3f, 0.0f };

    float shadow[4] = { 0.45f, 0.6f, 0.0f, 0.0f };
};

inline ToonSettings g_toonSettings;