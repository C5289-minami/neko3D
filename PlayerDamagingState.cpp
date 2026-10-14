#include "PlayerDamagingState.h"
#include "Player.h"
#include "DxPlus/Debug.h"

PlayerDamagingState::PlayerDamagingState(PlayerStateMachine& stateMachine, Player& player)
    : PlayerBaseState(stateMachine, player)
{
}

void PlayerDamagingState::Enter()
{
    remainingTime_ = Player::DamageRecoveryTime;
    player_.StopMove();
    // 被弾専用アニメーションないので、待機アニメーションを仮に使用する。
    player_.SetAnimation(PlayerAnimType::Idle, true);
}

void PlayerDamagingState::Tick(float deltaTime)
{
    player_.StopMove();
    remainingTime_ -= deltaTime;
    if (remainingTime_ <= 0.0f)
    {
        if (player_.IsDead())
            stateMachine_.ChangeState(PlayerStateType::Die);
        else if (!player_.IsGrounded() && player_.IsGravityEnabled())
            stateMachine_.ChangeState(PlayerStateType::NeutralJump);
        else if(player_.GetInput().isMoving)
			stateMachine_.ChangeState(PlayerStateType::Walk);
        else
			stateMachine_.ChangeState(PlayerStateType::Idle);
        return;
    }
    DxPlus::Debug::SetFormatString(L"state:damaging");
}

void PlayerDamagingState::Exit()
{
    player_.StopMove();
}
