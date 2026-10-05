#include "ToonSettings.h"

#include <fstream>
#include <nlohmann/json.hpp>

ToonSettings& ToonSettingsManager::Get()
{
    return g_toonSettings;
}

bool ToonSettingsManager::Save(const std::string& filePath)
{
    nlohmann::json json;

    json["shadowColor"] = {
        g_toonSettings.shadowColor[0],
        g_toonSettings.shadowColor[1],
        g_toonSettings.shadowColor[2]
    };

    json["thresholds"] = {
        g_toonSettings.thresholds[0],
        g_toonSettings.thresholds[1],
        g_toonSettings.thresholds[2]
    };

    json["shadow"] = {
        g_toonSettings.shadow[0],
        g_toonSettings.shadow[1]
    };

    std::ofstream file(filePath);

    if (!file)
    {
        return false;
    }

    file << json.dump(4);

    return true;
}

bool ToonSettingsManager::Load(const std::string& filePath)
{
    std::ifstream file(filePath);

    if (!file)
    {
        return false;
    }

    nlohmann::json json;
    file >> json;

    if (json.contains("shadowColor"))
    {
        for (int i = 0; i < 3; ++i)
        {
            g_toonSettings.shadowColor[i] =
                json["shadowColor"][i];
        }
    }

    if (json.contains("thresholds"))
    {
        for (int i = 0; i < 3; ++i)
        {
            g_toonSettings.thresholds[i] =
                json["thresholds"][i];
        }
    }

    if (json.contains("shadow"))
    {
        for (int i = 0; i < 2; ++i)
        {
            g_toonSettings.shadow[i] =
                json["shadow"][i];
        }
    }

    return true;
}

void ToonSettingsManager::Reset()
{
    g_toonSettings = ToonSettings{};
}