#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

// パラメータ構造体
struct PlayerParams {
    int hp = 100;
    float speed = 5.0f;
    int attack = 15;
};

struct EnemyParams {
    int hp = 50;
    float speed = 3.0f;
    int attack = 8;
};

struct GameParams {
    PlayerParams player;
    EnemyParams enemy;
};

// パラメータ読み込み関数
bool LoadParams(const std::string& filepath, GameParams& outParams) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        // ファイルがない場合はデフォルト値のまま続行
        return false;
    }

    try {
        json j;
        file >> j;

        // Playerの読み込み（キーがなくてもデフォルト値を採用）
        if (j.contains("player")) {
            auto& p = j["player"];
            outParams.player.hp = p.value("hp", outParams.player.hp);
            outParams.player.speed = p.value("speed", outParams.player.speed);
            outParams.player.attack = p.value("attack", outParams.player.attack);
        }

        // Enemyの読み込み
        if (j.contains("enemy")) {
            auto& e = j["enemy"];
            outParams.enemy.hp = e.value("hp", outParams.enemy.hp);
            outParams.enemy.speed = e.value("speed", outParams.enemy.speed);
            outParams.enemy.attack = e.value("attack", outParams.enemy.attack);
        }

        return true;
    }
    catch (const std::exception& e) {
        // コンマ忘れなどのJSON構文エラー時も落ちずに前回の値を保持
        return false;
    }
}