#include "PlayerUltimateAttackState.h"
#include "DxPlus/Debug.h"
#include "Player.h"

PlayerUltimateAttackState::PlayerUltimateAttackState(PlayerStateMachine& stateMachine, Player& player)
    : PlayerBaseState(stateMachine, player)
{
}

void PlayerUltimateAttackState::Enter()
{
    // ƒQ[ƒW‚ğÁ”ï‚µ‚Ä•KE‹Z‚ğŠJn
    if (!player_.TryUseUltimateGauge())
    {
        stateMachine_.ChangeState(PlayerStateType::Idle);
        return;
    }
    player_.StopMove();
    player_.SetAnimation(PlayerAnimType::Ult_Attack, false);
    player_.GetAnimation().Start();
    DxPlus::Debug::SetFormatString(L"state:ultimate_attack");
}

void PlayerUltimateAttackState::Tick(float deltaTime)
{
    player_.UpdateAnimation(deltaTime);
    if (player_.GetAnimation().IsFinished())
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
