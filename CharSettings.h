#pragma once

#include <string>
#include <vector>

struct CharData
{
    std::string name;   // 名前
    float maxHealth = 100.0f;  // 最大体力
    float moveSpeed = 5.0f;    // 移動速度
    float attackRatio = 1.0f;  // 攻撃倍率
    float attackSpeed = 1.0f;  // 攻撃速度
};

class CharSettingsManager
{
public:
    // JSONに保存
    static bool Save(
        const std::string& filePath = "./Data/Config/charData.json"
    );

    // JSONから読み込み
    static bool Load(
        const std::string& filePath = "./Data/Config/charData.json"
    );

    // キャラクター情報を初期化
    static void Reset();

    // キャラクター数を取得
    static size_t GetCount();

    // キャラクター情報を取得
    static const CharData& Get(size_t index);

    // 全キャラクター情報を取得
    static const std::vector<CharData>& GetAll();

private:
    static std::vector<CharData> charDataList;
};