#include "PlayerInput.h"
#include "DxPlus/DxPlus.h"

void PlayerInput::Update()
{
	using namespace DxPlus::Input;

	int button = GetButton(PLAYER1);

	// ˆÚ“®•ûŒü
	moveDir = Vec3(0.0f, 0.0f, 0.0f);
	if (button & BUTTON_UP) moveDir.z += 1.0f;
	if (button & BUTTON_DOWN) moveDir.z -= 1.0f;
	if (button & (BUTTON_LEFT | BUTTON_L1)) moveDir.x -= 1.0f;
	if (button & (BUTTON_RIGHT | BUTTON_R1)) moveDir.x += 1.0f;

	turn = 0.0f;
	isMoving = (moveDir.LengthSq() > 0.0f);
}
