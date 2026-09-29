#include "PlayerInput.h"
#include "DxPlus/DxPlus.h"

void PlayerInput::Update()
{
	using namespace DxPlus::Input;

	int button = GetButton(PLAYER1);
	int buttonDown = GetButtonDown(PLAYER1);

	// ˆÚ“®•ûŒü‚ÌŽæ“¾
	moveDir = Vec3(0.0f, 0.0f, 0.0f);

	// ‘OŒã
	if (GetButton(PLAYER1) & BUTTON_UP)    moveDir.z += 1.0f;
	if (GetButton(PLAYER1) & BUTTON_DOWN)  moveDir.z -= 1.0f;
	// ‰ñ“]
	turn = 0.0f;
	if (GetButton(PLAYER1) & BUTTON_LEFT) turn -= 1.0f;
	if (GetButton(PLAYER1) & BUTTON_RIGHT) turn += 1.0f;
	// ¶‰E
	if (GetButton(PLAYER1) & BUTTON_L1)  moveDir.x -= 1.0f;
	if (GetButton(PLAYER1) & BUTTON_R1) moveDir.x += 1.0f;	

	isMoving = (moveDir.LengthSq() > 0.0f);

}