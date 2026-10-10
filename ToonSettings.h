#pragma once

#include <string>

// シェーダーのトゥーン設定用
struct ToonSettings
{
    float shadowColor[4] = { 0.2f, 0.25f, 0.6f, 1.0f };

    float thresholds[4] = { 0.9f, 0.7f, 0.3f, 0.0f };

    float shadow[4] = { 0.45f, 0.6f, 0.0f, 0.0f };

    float LightDirection[4] = {};
};

inline ToonSettings g_toonSettings;


class ToonSettingsManager
{
public:
    static ToonSettings& Get();

    static bool Save(
        const std::string& filePath = "./Data/Config/toon.json"
    );

    static bool Load(
        const std::string& filePath = "./Data/Config/toon.json"
    );

    static void Reset();
};