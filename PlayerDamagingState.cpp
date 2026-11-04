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
    avoidTimer_ = AvoidTime;
    player_.StopMove();
    // 被弾専用アニメーションないので、待機アニメーションを仮に使用する。
    player_.SetAnimation(PlayerAnimType::Idle, true);
}

void PlayerDamagingState::Tick(float deltaTime)
{
    const float elapsed = (std::max)(0.0f, deltaTime);
    if (avoidTimer_ > 0.0f)
    {
        if (elapsed <= avoidTimer_ && player_.GetInput().jumpPressed &&
            !player_.CanGroundJump() && player_.CanDoubleJump())
        {
            avoidTimer_ = 0.0f;
			player_.ApplyAvoid();//被弾回避の処理を行う
            stateMachine_.ChangeState(PlayerStateType::DoubleJump);
            return;
        }

        // 受付が終わったフレームだけ、保留していたダメージを適用する。
        avoidTimer_ = (std::max)(0.0f, avoidTimer_ - elapsed);
        if (avoidTimer_ == 0.0f)
            player_.AddDamage();
    }

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
