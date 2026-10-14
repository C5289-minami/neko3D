#include "PlayerUltimateAttackState.h"
#include "DxPlus/Debug.h"
#include "Player.h"

PlayerUltimateAttackState::PlayerUltimateAttackState(PlayerStateMachine& stateMachine, Player& player)
    : PlayerBaseState(stateMachine, player)
{
}

void PlayerUltimateAttackState::Enter()
{
    // ゲージを消費して必殺技を開始
    if (!player_.TryUseUltimateGauge())
    {
        stateMachine_.ChangeState(PlayerStateType::Idle);
        return;
    }
    player_.StopMove();
    remainingTime_ = 1.0f; // 確認用に1秒だけ表示
    DxPlus::Debug::SetFormatString(L"state:ultimate_attack");
}

void PlayerUltimateAttackState::Tick(float deltaTime)
{
    remainingTime_ -= deltaTime;
    if (remainingTime_ <= 0.0f)
    {
        stateMachine_.ChangeState(player_.GetInput().isMoving
            ? PlayerStateType::Walk : PlayerStateType::Idle);
        return;
    }
    DxPlus::Debug::SetFormatString(L"state:ultimate_attack");
}

void PlayerUltimateAttackState::Exit()
{
    player_.StopMove();
    DxPlus::Debug::SetFormatString(L"PlayerUltimateAttackState::Exit");
}
