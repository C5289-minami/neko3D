#pragma once

#include <string>

struct OutlineSettings
{
    float radius = 2.0f;
    float depthStrength = 20.0f;
    float normalStrength = 5.0f;
    float threshold = 0.1f;
};

inline OutlineSettings g_outlineSettings;

class OutlineSettingsManager
{
public:
    static bool Save(
        const std::string& filePath = "./Data/Config/outline.json"
    );

    static bool Load(
        const std::string& filePath = "./Data/Config/outline.json"
    );

    static void Reset();
};