#include "PlayerClimbingState.h"
#include "DxPlus/Debug.h"
#include "Player.h"

PlayerClimbingState::PlayerClimbingState(PlayerStateMachine& stateMachine, Player& player)
    : PlayerBaseState(stateMachine, player)
{
}

void PlayerClimbingState::Enter()
{
    player_.StopMove();
    // 壁登り用のアニメーションを用意するまでは、既存の待機アニメーションを使う。
    player_.SetAnimation(PlayerAnimType::Idle, true);
    DxPlus::Debug::SetFormatString(L"PlayerClimbingState::Enter");
}

void PlayerClimbingState::Tick(float)
{
    const PlayerInput& input = player_.GetInput();
	Vec3 playerPos = player_.GetPosition();

    // ジャンプボタンを押している間、変更した座標をプレイヤーに反映する。
    DxPlus::Debug::SetFormatString(L"state:climbing");
    if(input.jumpPressing)
    {
		playerPos.y += 100.f/2.0f; // 上昇
        player_.SetPosition(playerPos);
    }

}

void PlayerClimbingState::Exit()
{
    player_.StopMove();
    DxPlus::Debug::SetFormatString(L"PlayerClimbingState::Exit");
}
