#pragma once

// 各列挙値はモデルのアニメーション番号として使う。
// モデル内のクリップ順と一致していることを確認する。
enum class PlayerStateType
{
    Idle = 0,       // 待機
    Walk,           // 歩行
    Kedukuroi,       // 毛づくろい
    NeutralJump,    // その場でジャンプ
    LungingJump,    // 移動しながらジャンプ
    Attack,         // 通常攻撃
    UltimateAttack, // 必殺技
    Die,            // 死亡
};