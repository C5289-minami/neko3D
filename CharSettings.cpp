#include "CharSettings.h"

#include <fstream>
#include <algorithm>
#include <nlohmann/json.hpp>

using json = nlohmann::json;


std::vector<CharData> CharSettingsManager::charDataList;

// 保存
bool CharSettingsManager::Save(const std::string& filePath)
{
    json j;

    j["characters"] = json::array();

    for (const auto& data : charDataList)
    {
        j["characters"].push_back({
            {"name", data.name},
            {"maxHealth", data.maxHealth},
            {"moveSpeed", data.moveSpeed},
            {"attackRatio", data.attackRatio},
            {"attackSpeed", data.attackSpeed}
            });
    }

    std::ofstream file(filePath);

    if (!file)
    {
        return false;
    }

    file << j.dump(4);
    return file.good();
}

// 読み込み
bool CharSettingsManager::Load(const std::string& filePath)
{
    std::ifstream file(filePath);

    if (!file)
    {
        return false;
    }

    try
    {
        json j;
        file >> j;

        if (!j.contains("characters") ||
            !j["characters"].is_array())
        {
            return false;
        }

        // 読み込みに成功したデータを一時保存
        std::vector<CharData> loadedData;

        for (const auto& item : j["characters"])
        {
            if (!item.is_object())
            {
                return false;
            }

            CharData data;

            if (item.contains("name") &&
                item["name"].is_string())
            {
                data.name = item["name"].get<std::string>();
            }

            if (item.contains("maxHealth") &&
                item["maxHealth"].is_number())
            {
                data.maxHealth = item["maxHealth"].get<float>();
            }

            if (item.contains("moveSpeed") &&
                item["moveSpeed"].is_number())
            {
                data.moveSpeed = item["moveSpeed"].get<float>();
            }

            if (item.contains("attackRatio") &&
                item["attackRatio"].is_number())
            {
                data.attackRatio = item["attackRatio"].get<float>();
            }

            if (item.contains("attackSpeed") &&
                item["attackSpeed"].is_number())
            {
                data.attackSpeed = item["attackSpeed"].get<float>();
            }

            loadedData.push_back(data);
        }

        // 全件読み込みに成功した場合のみ反映
        charDataList = std::move(loadedData);

        return true;
    }
    catch (const json::exception&)
    {
        return false;
    }
}

// 初期化
void CharSettingsManager::Reset()
{
    charDataList.clear();
}

// キャラクター数
size_t CharSettingsManager::GetCount()
{
    return charDataList.size();
}

// インデックス指定で取得
const CharData& CharSettingsManager::Get(size_t index)
{
    return charDataList.at(index);
}

// 全件取得
const std::vector<CharData>& CharSettingsManager::GetAll()
{
    return charDataList;
}