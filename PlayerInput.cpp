#include "PlayerInput.h"
#include "DxPlus/DxPlus.h"

void PlayerInput::Update()
{
	using namespace DxPlus::Input;

	int button = GetButton(PLAYER1);
	int buttonDown = GetButtonDown(PLAYER1);

	// 移動方向
	moveDir = Vec3(0.0f, 0.0f, 0.0f);
	if (button & BUTTON_UP) moveDir.z += 1.0f;
	if (button & BUTTON_DOWN) moveDir.z -= 1.0f;
	if (button & (BUTTON_LEFT | BUTTON_L1)) moveDir.x -= 1.0f;
	if (button & BUTTON_RIGHT) moveDir.x += 1.0f;

	jumpPressed = (buttonDown & BUTTON_TRIGGER1) != 0;
	// 左クリックの攻撃入力
	attackPressed = (buttonDown & BUTTON_TRIGGER3) != 0;
	ultimatePressed = (buttonDown & BUTTON_TRIGGER4) != 0;
	groomingPressed = (buttonDown & BUTTON_R1) != 0;

	isMoving = (moveDir.LengthSq() > 0.0f);
}
