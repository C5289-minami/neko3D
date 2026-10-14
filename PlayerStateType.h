#pragma once

// ステートの番号と、PlayerAnimTypeのアニメーション番号は別に管理する。
enum class PlayerStateType
{
    Idle = 0,       // 待機
    Walk,           // 歩行
    Kedukuroi,      // 毛づくろい
    NeutralJump,    // その場でジャンプ
    LungingJump,    // 移動しながらジャンプ
    Attack,         // 通常攻撃
    UltimateAttack, // 必殺技
    Die,            // 死亡
    Damaging,       // 被弾中
};
