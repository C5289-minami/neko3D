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
    // 壁登りの許可判定、ジャンプボタンの長押し判定、上下移動は後で追加する。
    DxPlus::Debug::SetFormatString(L"state:climbing");
}

void PlayerClimbingState::Exit()
{
    player_.StopMove();
    DxPlus::Debug::SetFormatString(L"PlayerClimbingState::Exit");
}
