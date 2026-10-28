#pragma once

namespace PlayerAnimType
{
    enum Type
    {
        Idle = 0,         //
        Walk,             //
		Grooming,        // けづくろい（Grooming）                    ・E
		Jump,     // 通常ジャンプ（Jump）       ・space
		Double_Jump,      // ダブルジャンプ。モデル内のアニメーション番号は維持する。
		Attack,           // 攻撃（Attack）                           ・left mouse button
		Ult_Attack,       // 必殺技（Ultimate Attack）                ・right mouse button
		Die,              // 死亡（Die）
    };
}
