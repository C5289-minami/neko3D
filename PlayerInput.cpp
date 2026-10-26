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

	// 前後と左右を同時に押した間は、どちらにも移動しない。
	//if (moveDir.x != 0.0f && moveDir.z != 0.0f)
	//{
	//	moveDir = {};
	//}

    // 押した瞬間の1フレームだけtrue。1回目・2回目のジャンプはこちらを使う。
    jumpPressed = (buttonDown & BUTTON_TRIGGER1) != 0;
    // 押している間ずっとtrue。柱を登るための長押しはこちらを使う。
    jumpPressing = (button & BUTTON_TRIGGER1) != 0;
	// 左クリックの攻撃入力
	attackPressed = (buttonDown & BUTTON_TRIGGER3) != 0;
	ultimatePressed = (buttonDown & BUTTON_TRIGGER4) != 0;
	groomingPressed = (buttonDown & BUTTON_R1) != 0;

	isMoving = (moveDir.LengthSq() > 0.0f);
}
