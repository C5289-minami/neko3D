#pragma once
#include "Vector3.h"
	
struct PlayerInput
{
	Vec3 moveDir{};

	float turn{ 0.0f };

	bool isMoving{ false };
	bool run{ false };
	bool jumpPressed{ false };
	bool attackPressed{ false };

	void Update();
};


