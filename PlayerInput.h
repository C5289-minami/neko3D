#pragma once
#include "Vector3.h"
	
struct PlayerInput
{
	Vec3 moveDir{};

	float turn{ 0.0f };

	bool isMoving{ false };
	bool run{ false };
	bool jumpPressed{ false };
	bool jumpPressing{ false };
	bool enterPressed{ false };  // Enter / STARTを押した瞬間
	bool enterPressing{ false }; // Enter / STARTを押している間
	bool skipPressed{ false }; // 右Shiftキーを押した瞬間
    bool RkeyPressed{ false };  // Rキーを押した瞬間
	bool shiftRKeyPressed{ false }; // Shift + Rキーを押した瞬間（デバッグ用）
	bool attackPressed{ false };
	bool ultimatePressed{ false };
#ifdef _DEBUG
	bool debugUltimatePressed{ false };
#endif
	bool groomingPressed{ false };

	void Update();
};


