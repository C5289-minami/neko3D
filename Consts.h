// =============================
// Core/Consts.h
// =============================
#pragma once
#include "DxPlus/DxPlus.h"

namespace Const
{
    // ===== プレイヤー関連 =====

    constexpr float PLAYER_TURN_RATE_90 = 6.0f;
    constexpr float PLAYER_TURN_RATE_180 = 120.0f;
    constexpr float PLAYER_ROTATE_SPEED = DxPlus::Deg2Rad * 180.0f;
    constexpr float PLAYER_JUMP_SPEED = 1000.0f;
    constexpr float PLAYER_MOVE_SPEED = 200.0f;
    constexpr float PLAYER_HEIGHT = 180.0f;
    constexpr float PLAYER_SKIN = 1.0f;
    constexpr float PLAYER_RADIUS = 60.0f;

    // ===== 敵関連 =====
    constexpr float BOSS_MODEL_SCALE = 1.0f;
    constexpr float BOSS_HIDDEN_POSITION_Y = -0.0f;
    constexpr float BOSS_IDLE_MOTION_DURATION = 1.0f;
    constexpr float BOSS_WALK_MOTION_DURATION = 3.0f;
    constexpr float BOSS_MOVE_SPEED = 200.0f;

    // ===== 物理共通 =====
    constexpr float GRAVITY = 2000.0f;

    // ===== ジャンプ関連 =====

    // ===== ゲーム内共通 =====
    constexpr int FPS_CAP = 480;
    constexpr float EPS = 1e-3f;
    constexpr float ANIM_FPS = 30.0f;
 
}
