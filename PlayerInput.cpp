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

	// 斜め入力も受け付け、縦横だけの入力と移動速度を揃える。
	moveDir = moveDir.Normalized();

	//if (moveDir.x != 0.0f && moveDir.z != 0.0f)
	//{
	//	moveDir = {};
	//}

    // 押した瞬間の1フレームだけtrue。1回目・2回目のジャンプはこちらを使う。
    jumpPressed = (buttonDown & BUTTON_TRIGGER1) != 0;
    // 押している間ずっとtrue。柱を登るための長押しはこちらを使う。
    jumpPressing = (button & BUTTON_TRIGGER1) != 0;

    // Shiftキー押下判定（左右Shift対応）
    const bool shiftPressing =
        DxLib::CheckHitKey(KEY_INPUT_LSHIFT) != 0 ||
        DxLib::CheckHitKey(KEY_INPUT_RSHIFT) != 0;

    // Rキーの入力（単独押しとShift同時押しを分ける）
    const bool rKeyDown = (buttonDown & BUTTON_R) != 0;
    RkeyPressed = rKeyDown && !shiftPressing;
    shiftRKeyPressed = rKeyDown && shiftPressing;
	
	// 左クリックの攻撃入力
	//attackPressed = (buttonDown & BUTTON_TRIGGER3) != 0;
	attackPressed = (buttonDown & BUTTON_START) != 0;	//Enterキーで攻撃するように変更
	ultimatePressed = (buttonDown & BUTTON_TRIGGER4) != 0;
#ifdef _DEBUG
	debugUltimatePressed = attackPressed && DxLib::CheckHitKey(KEY_INPUT_LSHIFT) != 0;
	ultimatePressed = ultimatePressed || debugUltimatePressed;
	attackPressed = attackPressed && !debugUltimatePressed;
#endif
	groomingPressed = (buttonDown & BUTTON_R1) != 0;

	isMoving = (moveDir.LengthSq() > 0.0f);
}
