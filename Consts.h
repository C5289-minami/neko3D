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

	constexpr float PLAYER_CLIMBING_SPEED = 300.0f;
    constexpr float PLAYER_WALL_CLIMB_WAIT = 0.2f;
    constexpr float PLAYER_WALL_SLIDE_SPEED = 600.0f;
    constexpr float PLAYER_ELEVATOR_SPEED = 600.0f;
    constexpr float PLAYER_ELEVATOR_TOP_OFFSET = 10.0f;
    constexpr float PLAYER_ELEVATOR_EXIT_JUMP_SPEED = 350.0f;
    
    constexpr float PLAYER_JUMP_SPEED = 800.0f;
    constexpr float PLAYER_MOVE_SPEED = 350.0f;
    // 突進ジャンプの仮の速度
    // constexpr float PLAYER_LUNGING_MOVE_SPEED = 600.0f;
    constexpr float PLAYER_DOUBLE_JUMP_SPEED = 500.0f;
    constexpr float PLAYER_WALL_JUMP_MOVE_SPEED = 600.0f;
    constexpr float PLAYER_WALL_JUMP_SPEED = 800.0f;

    constexpr float PLAYER_HEIGHT = 60.0f;
    constexpr float PLAYER_SKIN = 1.0f;
    constexpr float PLAYER_RADIUS = 30.0f;
    constexpr float PLAYER_ATTACK_BALL_LIFETIME = 0.6f;
    constexpr float PLAYER_ATTACK_BALL_RADIUS = PLAYER_RADIUS;
    constexpr float PLAYER_ATTACK_BALL_HEIGHT_MULTIPLIER = 0.5f;
    constexpr float PLAYER_ATTACK_BALL_FORWARD_OFFSET = PLAYER_ATTACK_BALL_RADIUS;

  
    // 壁登りの検出専用。移動を止めるカプセルの半径は変えない。
    constexpr float PLAYER_CLIMB_DETECTION_MARGIN = 10.0f;
    // 足元が柱の上端に近づいたとき、登り中の高さを保持する範囲。
    constexpr float PLAYER_CLIMB_TOP_HOLD_DISTANCE = PLAYER_RADIUS;

    // ===== 敵関連 =====
    constexpr float BOSS_MODEL_SCALE = 1.0f;
    constexpr float BOSS_HIDDEN_POSITION_Y = -0.0f;
    constexpr float BOSS_IDLE_MOTION_DURATION = 1.0f;
    constexpr float BOSS_WALK_MOTION_DURATION = 3.0f;
    constexpr float BOSS_MOVE_SPEED = 200.0f;

    // ボスの両手攻撃が閉じるまでに移動する弧の長さ。
    constexpr float BOSS_BETWEEN_HAND_ATTACK_SIDE = 400.0f;
    // ボスの手の攻撃判定の半径。
    constexpr float BOSS_HAND_ATTACK_HIT_RADIUS = 90.0f;

    // ===== 物理共通 =====
    constexpr float GRAVITY = 2000.0f;

    // ===== ジャンプ関連 =====

    // ===== ゲーム内共通 =====
    constexpr int FPS_CAP = 480;
    constexpr float EPS = 1e-6f;
    constexpr float ANIM_FPS = 24.0f;

    // ===== カメラ =====
    constexpr float CAMERA_NEAR = 10.0f;
    constexpr float CAMERA_FAR = 20000.0f;
 
}
