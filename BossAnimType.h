#pragma once

namespace BossAnimType
{
	enum Type
	{
		Idle = 0,
		Walk,
		Run,
		Jump,
		Landing,
		Back,
		StrafeLeft,
		StrafeRight,
		Slash,
		Slash2,
		Spin = 10,	
		Combo,
		Kick,
		Shield,
		Impact,
		PowerUp, 
		Death,
		Count // アニメーションの総数を表すためのカウント
	};
}