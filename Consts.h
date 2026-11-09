// =============================
// Core/Consts.h
// =============================
#pragma once
#include "DxPlus/DxPlus.h"

namespace Const
{
    constexpr int MAX_VOICE = 255;

    // ===== プレイヤー関連 =====

    // 戦闘の調整値。HPとダメージは整数で、時間の単位は秒。
    constexpr int PLAYER_MAX_HEALTH = 5;
    constexpr int PLAYER_NORMAL_ATTACK_DAMAGE = 1;
    constexpr float PLAYER_DAMAGE_RECOVERY_SECONDS = 0.5f;
    constexpr float PLAYER_AVOID_WINDOW_SECONDS = 0.3f;
    constexpr float PLAYER_AVOID_INVINCIBILITY_SECONDS = 0.2f;

    constexpr float PLAYER_ULTIMATE_GAUGE_MAX = 100.0f;
	constexpr float PLAYER_ULTIMATE_GAUGE_INITIAL = 0.0f;   // ゲーム開始時のゲージ量
	constexpr float PLAYER_ULTIMATE_GAUGE_AFTER_USE = 0.0f; // 必殺技使用後のゲージ量
    // 戦闘中の1秒あたりの増加量と、通常攻撃1回の命中で増える量。
    constexpr float PLAYER_ULTIMATE_GAUGE_PER_SECOND = 3.0f;
    constexpr float PLAYER_ULTIMATE_GAUGE_PER_HIT = 20.0f;

    // 必殺技の弾。速度は1秒あたりの移動距離。
    constexpr int PLAYER_ULTIMATE_DAMAGE = 2;
    constexpr float PLAYER_ULTIMATE_PROJECTILE_SPEED = 1500.0f;
    constexpr float PLAYER_ULTIMATE_PROJECTILE_RADIUS = 45.0f;
    constexpr float PLAYER_ULTIMATE_PROJECTILE_LIFETIME_SECONDS = 3.0f;

    static_assert(PLAYER_MAX_HEALTH > 0);
    static_assert(PLAYER_ULTIMATE_DAMAGE >= 0);
    static_assert(PLAYER_ULTIMATE_PROJECTILE_SPEED > 0.0f &&
        PLAYER_ULTIMATE_PROJECTILE_RADIUS > 0.0f &&
        PLAYER_ULTIMATE_PROJECTILE_LIFETIME_SECONDS > 0.0f);
    static_assert(PLAYER_ULTIMATE_GAUGE_MAX > 0.0f);
    static_assert(PLAYER_ULTIMATE_GAUGE_INITIAL >= 0.0f &&
        PLAYER_ULTIMATE_GAUGE_INITIAL <= PLAYER_ULTIMATE_GAUGE_MAX);
    static_assert(PLAYER_ULTIMATE_GAUGE_AFTER_USE >= 0.0f &&
        PLAYER_ULTIMATE_GAUGE_AFTER_USE < PLAYER_ULTIMATE_GAUGE_MAX);
    static_assert(PLAYER_DAMAGE_RECOVERY_SECONDS >= 0.0f &&
        PLAYER_AVOID_WINDOW_SECONDS >= 0.0f &&
        PLAYER_AVOID_INVINCIBILITY_SECONDS >= 0.0f);

    constexpr float PLAYER_TURN_RATE_90 = 6.0f;
    constexpr float PLAYER_TURN_RATE_180 = 120.0f;
    constexpr float PLAYER_ROTATE_SPEED = DxPlus::Deg2Rad * 180.0f;

	constexpr float PLAYER_CLIMBING_SPEED = 300.0f;
    constexpr float PLAYER_WALL_CLIMB_WAIT = 0.2f;
    constexpr float PLAYER_WALL_SLIDE_SPEED = 600.0f;
    constexpr float PLAYER_ELEVATOR_SPEED = 350.0f;
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
    constexpr float PLAYER_FALL_RECOVERY_DISTANCE = 400.0f*1.7f;//落ちる幅の計算のために追加し
	constexpr float PLAYER_FALL_RECOVERY_HEIGHT_OFFSET = 10.0f;//落ちる幅の計算のために追加し
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
    constexpr int BOSS_MAX_HEALTH = 4;
    constexpr int BOSS_CLAP_DAMAGE = 1;
    constexpr int BOSS_SLAM_DAMAGE = 1;
    constexpr int BOSS_TAIL_WHIP_DAMAGE = 1;

    constexpr float BOSS_DOWN_GAUGE_MAX = 100.0f;
    constexpr float BOSS_DOWN_GAUGE_HIT_REDUCTION = 20.0f;
    // フレーム時間に応じた減少は、ボスがダメージを受けたときだけ適用する。
    constexpr float BOSS_DOWN_GAUGE_HIT_FRAME_REDUCTION_PER_SECOND = 10.0f;
    constexpr float BOSS_DOWN_GAUGE_LONG_FRAME_SECONDS = 5.0f;
    constexpr float BOSS_DOWN_GAUGE_LONG_FRAME_REDUCTION = 5.0f;
    constexpr float BOSS_FLINCH_HOLD_SECONDS = 5.0f;

    static_assert(BOSS_MAX_HEALTH > 0 && BOSS_DOWN_GAUGE_MAX > 0.0f);
    static_assert(PLAYER_NORMAL_ATTACK_DAMAGE >= 0 && BOSS_CLAP_DAMAGE >= 0 &&
        BOSS_SLAM_DAMAGE >= 0 && BOSS_TAIL_WHIP_DAMAGE >= 0);

    constexpr float BOSS_MODEL_SCALE = 1.0f;
	// ボスのダウン中の攻撃判定の半径。ステージ中心を基準とするワールド座標の大きさ。
    constexpr float BOSS_DOWN_HURT_RADIUS = 685.0f;//Flich
    // ボス本体のカプセル。ステージ中心を基準とするワールド座標の大きさ。
    constexpr float BOSS_BODY_RADIUS = 240.0f;     // 本体の半径（太さ）。
    constexpr float BOSS_BODY_HEIGHT = 1150.0f;     // 上下の丸い部分を含む全高。
    constexpr float BOSS_BODY_Y_OFFSET = -350.0f;     // ステージ位置を基準にした底の高さ。
    // ジャンプ接触で減らすダウンゲージ量。2階からの二段目は増量する。
    constexpr float BOSS_JUMP_DOWN_DAMAGE = 20.0f;
    constexpr float BOSS_UPPER_DOUBLE_JUMP_DOWN_DAMAGE = 40.0f;
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
