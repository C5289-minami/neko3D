#pragma once
#include "TransformSettings.h"

namespace TransformDefaults
{
    // 保存ボタンで、この範囲の初期値を書き換える
    // BEGIN_TRANSFORM(game/player)
    inline constexpr ObjectTransform GamePlayer{
        { 100.0f, 100.0f, 100.0f },
        { 0.0f, 36.0f, -521.0f },
        { 0.0f, 0.0f, 0.0f }
    };
    // END_TRANSFORM(game/player)

    // BEGIN_TRANSFORM(game/boss)
    inline constexpr ObjectTransform GameBoss{
        { 1.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f }
    };
    // END_TRANSFORM(game/boss)

    // BEGIN_TRANSFORM(boss_test/player)
    inline constexpr ObjectTransform BossTestPlayer{
        { 141.800003f, 100.0f, 100.0f },
        { 250.0f, 205.0f, 300.0f },
        { 0.0f, -0.129999995f, 0.0f }
    };
    // END_TRANSFORM(boss_test/player)

    // BEGIN_TRANSFORM(boss_test/boss)
    inline constexpr ObjectTransform BossTestBoss{
        { 1.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f }
    };
    // END_TRANSFORM(boss_test/boss)

    struct Entry
    {
        const char* id;
        const char* name;
        const ObjectTransform* value;
    };

    inline const Entry Entries[] = {
        { "game/player", "GamePlayer", &GamePlayer },
        { "game/boss", "GameBoss", &GameBoss },
        { "boss_test/player", "BossTestPlayer", &BossTestPlayer },
        { "boss_test/boss", "BossTestBoss", &BossTestBoss }
    };
}
