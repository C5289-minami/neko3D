#include "OutlineSettings.h"

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <nlohmann/json.hpp>

OutlineSettings g_outlineSettings{};
OutlineSettingsTableBuffer g_outlineSettingsTable{};

namespace
{
    constexpr float MinRadius = 1.0f;
    constexpr float MaxRadius = 5.0f;
    constexpr float MinValue = 0.0f;
    constexpr float MaxValue = 1.0f;
    constexpr float MaxDepthStrength = 50.0f;
    constexpr float MaxNormalStrength = 20.0f;

    template <typename JsonType>
    void LoadFloat(
        const JsonType& json,
        const char* key,
        float& value,
        float minimum,
        float maximum)
    {
        const auto it = json.find(key);

        if (it != json.end() && it->is_number())
        {
            value = std::clamp(
                it->get<float>(),
                minimum,
                maximum);
        }
    }
}

bool OutlineSettingsManager::Save(const std::string& filePath)
{
    nlohmann::json json;

    json["outerRadius"] = g_outlineSettings.outerRadius;
    json["depthStrength"] = g_outlineSettings.depthStrength;
    json["outerOpacity"] = g_outlineSettings.outerOpacity;

    json["innerRadius"] = g_outlineSettings.innerRadius;
    json["normalStrength"] = g_outlineSettings.normalStrength;
    json["normalThreshold"] = g_outlineSettings.normalThreshold;
    json["innerOpacity"] = g_outlineSettings.innerOpacity;

    json["outlineColor"] = {
        g_outlineSettings.outlineColor[0],
        g_outlineSettings.outlineColor[1],
        g_outlineSettings.outlineColor[2],
        g_outlineSettings.outlineColor[3]
    };

    std::ofstream file(filePath);

    if (!file)
    {
        return false;
    }

    file << json.dump(4);
    return file.good();
}

bool OutlineSettingsManager::Load(const std::string& filePath)
{
    std::ifstream file(filePath);

    if (!file)
    {
        return false;
    }

    try
    {
        nlohmann::json json;
        file >> json;

        OutlineSettings loaded{};

        // 以前の設定ファイルとの互換性
        float legacyRadius = loaded.outerRadius;
        LoadFloat(
            json,
            "radius",
            legacyRadius,
            MinRadius,
            MaxRadius);

        loaded.outerRadius = legacyRadius;
        loaded.innerRadius = legacyRadius;

        LoadFloat(
            json,
            "outerRadius",
            loaded.outerRadius,
            MinRadius,
            MaxRadius);

        LoadFloat(
            json,
            "depthStrength",
            loaded.depthStrength,
            MinValue,
            MaxDepthStrength);

        LoadFloat(
            json,
            "outerOpacity",
            loaded.outerOpacity,
            MinValue,
            MaxValue);

        LoadFloat(
            json,
            "innerRadius",
            loaded.innerRadius,
            MinRadius,
            MaxRadius);

        LoadFloat(
            json,
            "normalStrength",
            loaded.normalStrength,
            MinValue,
            MaxNormalStrength);

        LoadFloat(
            json,
            "normalThreshold",
            loaded.normalThreshold,
            MinValue,
            MaxValue);

        LoadFloat(
            json,
            "innerOpacity",
            loaded.innerOpacity,
            MinValue,
            MaxValue);

        const auto colorIt = json.find("outlineColor");

        if (colorIt != json.end() && colorIt->is_array())
        {
            const size_t count = std::min<std::size_t>(
                colorIt->size(),
                4);

            for (size_t i = 0; i < count; ++i)
            {
                if ((*colorIt)[i].is_number())
                {
                    loaded.outlineColor[i] = std::clamp(
                        (*colorIt)[i].get<float>(),
                        MinValue,
                        MaxValue);
                }
            }
        }

        g_outlineSettings = loaded;
        return true;
    }
    catch (const nlohmann::json::exception&)
    {
        return false;
    }
}

void OutlineSettingsManager::Reset()
{
    g_outlineSettings = OutlineSettings{};
}