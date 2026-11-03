#include "PlayerDamagingState.h"
#include "PlayerDoubleJumpState.h"
#include "Player.h"
#include "DxPlus/Debug.h"

PlayerDamagingState::PlayerDamagingState(PlayerStateMachine& stateMachine, Player& player)
    : PlayerBaseState(stateMachine, player)
{
}

void PlayerDamagingState::Enter()
{
    remainingTime_ = Player::DamageRecoveryTime;
    avoidTimer_ = PlayerDoubleJumpState::AvoidTime;
    player_.StopMove();
    // 被弾専用アニメーションないので、待機アニメーションを仮に使用する。
    player_.SetAnimation(PlayerAnimType::Idle, true);
}

void PlayerDamagingState::Tick(float deltaTime)
{
    const float elapsed = (std::max)(0.0f, deltaTime);
    if (avoidTimer_ > 0.0f && elapsed <= avoidTimer_ &&
        player_.GetInput().jumpPressed && !player_.CanGroundJump() &&
        player_.CanDoubleJump())
    {
        player_.ApplyAvoid(true);
        avoidTimer_ = 0.0f;
        stateMachine_.ChangeState(PlayerStateType::DoubleJump);
        return;
    }
    avoidTimer_ = (std::max)(0.0f, avoidTimer_ - elapsed);

    player_.StopMove();
    remainingTime_ -= elapsed;
    if (remainingTime_ <= 0.0f)
    {
        if (player_.IsDead())
            stateMachine_.ChangeState(PlayerStateType::Die);
        else if (!player_.IsGrounded() && player_.IsGravityEnabled())
            stateMachine_.ChangeState(PlayerStateType::Jump);
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
