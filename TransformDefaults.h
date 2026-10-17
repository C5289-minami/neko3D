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
        { 100.0f, 100.0f, 100.0f },
        { 445.020233f, 13.2437506f, 295.792603f },
        { 0.0f, 2.4982214f, 0.0f }
    };
    // END_TRANSFORM(boss_test/player)

    // BEGIN_TRANSFORM(boss_test/boss)
    inline constexpr ObjectTransform BossTestBoss{
        { 1.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f }
    };
    // END_TRANSFORM(boss_test/boss)

    // BEGIN_TRANSFORM(motion_test/player)
    inline constexpr ObjectTransform MotionTestPlayer{
        { 100.0f, 100.0f, 100.0f },
        { 0.0f, 36.0f, -521.0f },
        { 0.0f, 0.0f, 0.0f }
    };
    // END_TRANSFORM(motion_test/player)

    // BEGIN_TRANSFORM(motion_test/room)
    inline constexpr ObjectTransform MotionTestRoom{
        { 1.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f }
    };
    // END_TRANSFORM(motion_test/room)

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
        { "boss_test/boss", "BossTestBoss", &BossTestBoss },
        { "motion_test/player", "MotionTestPlayer", &MotionTestPlayer },
        { "motion_test/room", "MotionTestRoom", &MotionTestRoom }
    };
}
