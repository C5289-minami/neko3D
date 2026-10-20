#include "PlayerClimbingState.h"
#include "DxPlus/Debug.h"
#include "Player.h"
#include <algorithm>

PlayerClimbingState::PlayerClimbingState(PlayerStateMachine& stateMachine, Player& player)
    : PlayerBaseState(stateMachine, player)
{
}

void PlayerClimbingState::Enter()
{
    player_.StopMove();
    player_.SetVerticalSpeed(0.0f);
    pushTime_ = 0.0f;
    pushDirection_ = {};
    // 壁登り用のアニメーションを用意するまでは、既存の待機アニメーションを使う。
    player_.SetAnimation(PlayerAnimType::Idle, true);
    DxPlus::Debug::SetFormatString(L"PlayerClimbingState::Enter");
}

void PlayerClimbingState::Tick(float deltaTime)
{
    const PlayerInput& input = player_.GetInput();
    if (input.jumpPressed && player_.IsTouchingWall() && player_.IsGravityEnabled())
    {
        stateMachine_.ChangeState(PlayerStateType::WallJump);
        return;
    }

    const float wallDirection = Vec3::Dot(
        player_.GetWorldMoveDirection(input.moveDir),
        player_.GetWallNormal());
    const bool pushingWall = input.isMoving && wallDirection < -0.25f;
    // 地上でも壁を押している間は、登り始めるまでの時間を数え続ける。
    
    if (!player_.IsTouchingWall() || !player_.IsGravityEnabled() ||
        (input.isMoving && wallDirection > 0.5f) || (player_.IsGrounded() && !pushingWall))
    {
        stateMachine_.ChangeState(player_.IsGrounded() || !player_.IsGravityEnabled()
            ? (input.isMoving ? PlayerStateType::Walk : PlayerStateType::Idle)
            : PlayerStateType::NeutralJump);

        return;
    }

    // 入力は方向と時間の判定に使い、カウント中も横移動は加えない。
    player_.StopMove();
    if (pushingWall)
    {
        const Vec3 direction = input.moveDir.Normalized();
        // 約60度以内の違いは同じ方向として扱う。

        if (Vec3::Dot(direction, pushDirection_) < 0.5f)
        {
            pushTime_ = 0.0f;
            pushDirection_ = direction;
        }
        pushTime_ += deltaTime;
    }
    else
    {
        pushTime_ = 0.0f;
        pushDirection_ = {};
    }

    //bool ClimbingSpeed = player_.IsGrounded() || player_.IsNearWallTop()
    //                            ? 0.0f : -Const::PLAYER_WALL_SLIDE_SPEED;
  
    // 頂上付近では入力を離しても高さを保つ。押し続けても上端を越えて上昇しない。
    if(player_.IsGrounded() || player_.IsNearWallTop())
    {
        float ClimbingSpeed = Const::PLAYER_CLIMBING_SPEED;
        if (pushTime_ >= Const::PLAYER_WALL_CLIMB_WAIT && deltaTime > 0.0f)
        {
            float MAXHeight = (std::max)(0.0f, player_.GetWallTopY() - player_.GetPosition().y);
            ClimbingSpeed = (std::min)(Const::PLAYER_CLIMBING_SPEED, MAXHeight / deltaTime);
        }
        player_.SetVerticalSpeed(ClimbingSpeed);
	}
    DxPlus::Debug::SetFormatString(L"state:climbing");
}

void PlayerClimbingState::Exit()
{
    player_.StopMove();
    player_.SetVerticalSpeed(0.0f);
    DxPlus::Debug::SetFormatString(L"PlayerClimbingState::Exit");
}
