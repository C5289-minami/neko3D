#pragma once

// =================================
// 攻撃状態の種類
// =================================

enum class AttackStateType
{
	None,
	PreAction,   // 予備動作（溜め・構え）
	Aiming,      // 狙っている（ターゲットロック）
	Attack,      // 攻撃本体
	Recovery,    // 余韻（硬直・攻撃受付）	
	Return       // 戻り（ニュートラルへ復帰）
};
