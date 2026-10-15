#pragma once

namespace PlayerAnimType
{
    enum Type
    {
        Idle = 0,         //
        Walk,             //
		Kedukuroi,        // けづくろい（Grooming）                    ・E
		neutral_Jump,     // ニュートラルジャンプ（Neutral Jump）       ・space
		Lunging_Jump,     //　ランジジャンプ（Lunging Jump）           ・WorAorSorD+Space
		Attack,           // 攻撃（Attack）                           ・left mouse button
		Ult_Attack,       // 必殺技（Ultimate Attack）                ・right mouse button
		Die,              // 死亡（Die）
    };
}
