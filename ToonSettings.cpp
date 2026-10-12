#include "ToonSettings.h"
#include "DxLib.h"
#include <cmath>

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

    json["LightDirection"] = {
        g_toonSettings.LightDirection[0],
        g_toonSettings.LightDirection[1],
        g_toonSettings.LightDirection[2]
	};
    json["HalftoneColor"] = {
    g_toonSettings.halftoneColor[0],
    g_toonSettings.halftoneColor[1],
    g_toonSettings.halftoneColor[2],
    g_toonSettings.halftoneColor[3]
    };

    json["HalftoneSettings"] = {
        g_toonSettings.halftoneSettings[0],
        g_toonSettings.halftoneSettings[1],
        g_toonSettings.halftoneSettings[2],
        g_toonSettings.halftoneSettings[3]
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

    if (json.contains("LightDirection"))
    {
        for (int i = 0; i < 3; ++i)
        {
            g_toonSettings.LightDirection[i] =
                json["LightDirection"][i];
        }

        const float x = g_toonSettings.LightDirection[0];
        const float y = g_toonSettings.LightDirection[1];
        const float z = g_toonSettings.LightDirection[2];

        const float length = std::sqrt(x * x + y * y + z * z);

        // 有効な方向ベクトルならDxLibにも反映
        if (length > 0.001f)
        {
            DxLib::SetLightDirection(VGet(
                x / length,
                y / length,
                z / length
            ));
        }
    }

    if (json.contains("HalftoneColor"))
    {
        for (int i = 0; i < 4; ++i)
        {
            g_toonSettings.halftoneColor[i] =
                json["HalftoneColor"][i];
        }
	}

    if (json.contains("HalftoneSettings"))
    {
        for (int i = 0; i < 4; ++i)
        {
            g_toonSettings.halftoneSettings[i] =
                json["HalftoneSettings"][i];
        }
	}


    return true;
}

void ToonSettingsManager::Reset()
{
    g_toonSettings = ToonSettings{};
}