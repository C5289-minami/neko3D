#include "OutlineSettings.h"

#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>

bool OutlineSettingsManager::Save(const std::string& filePath)
{
    nlohmann::json json;

    json["radius"] = g_outlineSettings.radius;
    json["depthStrength"] = g_outlineSettings.depthStrength;
    json["normalStrength"] = g_outlineSettings.normalStrength;
    json["threshold"] = g_outlineSettings.threshold;

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

        if (json.contains("radius") &&
            json["radius"].is_number())
        {
            g_outlineSettings.radius =
                std::clamp(json["radius"].get<float>(), 0.0f, 5.0f);
        }

        if (json.contains("depthStrength") &&
            json["depthStrength"].is_number())
        {
            g_outlineSettings.depthStrength =
                std::clamp(json["depthStrength"].get<float>(), 0.0f, 50.0f);
        }

        if (json.contains("normalStrength") &&
            json["normalStrength"].is_number())
        {
            g_outlineSettings.normalStrength =
                std::clamp(json["normalStrength"].get<float>(), 0.0f, 20.0f);
        }

        if (json.contains("threshold") &&
            json["threshold"].is_number())
        {
            g_outlineSettings.threshold =
                std::clamp(json["threshold"].get<float>(), 0.0f, 1.0f);
        }

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