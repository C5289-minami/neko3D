#include "PlayerAttackState.h"
#include "DxPlus/Debug.h"
#include "Player.h"

PlayerAttackState::PlayerAttackState(PlayerStateMachine& stateMachine, Player& player)
    : PlayerBaseState(stateMachine, player)
{
}

void PlayerAttackState::Enter()
{
    player_.StopMove();
    DxPlus::Debug::SetFormatString(L"PlayerAttackState::Enter");
}

void PlayerAttackState::Tick(float)
{
    const PlayerInput& input = player_.GetInput();
    // ゲージ100で右クリックすると必殺技
    if (input.ultimatePressed && player_.CanUseUltimate())
    {
        stateMachine_.ChangeState(PlayerStateType::UltimateAttack);
        return;
    }
    // 移動入力で歩行に戻る
    if (input.isMoving)
    {
        stateMachine_.ChangeState(PlayerStateType::Walk);
        return;
    }
    // もう一度左クリックで待機に戻る
    if (input.attackPressed)
    {
        stateMachine_.ChangeState(PlayerStateType::Idle);
        return;
    }
    DxPlus::Debug::SetFormatString(L"state:attack");
}

void PlayerAttackState::Exit()
{
    player_.StopMove();
    DxPlus::Debug::SetFormatString(L"PlayerAttackState::Exit");
}
