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
    // pushTime_ = 0.0f;
    // pushDirection_ = {};
    // 壁登り用のアニメーションを用意するまでは、既存の待機アニメーションを使う。
    player_.SetAnimation(PlayerAnimType::Idle, true);
    DxPlus::Debug::SetFormatString(L"PlayerClimbingState::Enter");
}

void PlayerClimbingState::Tick(float deltaTime)
{
    // 長押しを離したら終了する。再開は共通の接触判定とSpace入力で行う。
    if (!player_.IsPillarElevatorActive() || !player_.IsGravityEnabled() ||
        !player_.GetInput().jumpPressing)
    {
        stateMachine_.ChangeState(player_.IsGrounded() || !player_.IsGravityEnabled()
            ? (player_.GetInput().isMoving ? PlayerStateType::Walk : PlayerStateType::Idle)
            : PlayerStateType::Jump);
        return;
    }
    if (player_.UpdatePillarElevator(deltaTime))
    {
        stateMachine_.ChangeState(PlayerStateType::Jump);
        // Exitで速度をリセットした後に、小さなジャンプの初速を設定する。
        player_.StartPillarExitJump();
    }
}

// 旧仕様：移動入力で壁を登る処理。
// void PlayerClimbingState::Tick(float deltaTime)
// {
//     const PlayerInput& input = player_.GetInput();
//
//     // 移動入力を離したら壁登りを終了し、空中では通常の重力で落下する。
//     if (!input.isMoving)
//     {
//         if (player_.IsGrounded() || !player_.IsGravityEnabled())
//         {
//             stateMachine_.ChangeState(PlayerStateType::Idle);
//         }
//         else
//         {
//             stateMachine_.ChangeState(PlayerStateType::Jump);
//         }
//         return;
//     }
//
//     if (input.jumpPressed && player_.IsTouchingWall() && player_.IsGravityEnabled())
//     {
//         stateMachine_.ChangeState(PlayerStateType::WallJump);
//         return;
//     }
//
//     const float wallDirection = Vec3::Dot(
//         player_.GetWorldMoveDirection(input.moveDir),
//         player_.GetWallNormal());
//     const bool pushingWall = input.isMoving && wallDirection < -0.25f;
//     // 地上でも壁を押している間は、登り始めるまでの時間を数え続ける。
//
//     if (!player_.IsTouchingWall() || !player_.IsGravityEnabled() ||
//         (input.isMoving && wallDirection > 0.5f) || (player_.IsGrounded() && !pushingWall))
//     {
//         stateMachine_.ChangeState(player_.IsGrounded() || !player_.IsGravityEnabled()
//             ? (input.isMoving ? PlayerStateType::Walk : PlayerStateType::Idle)
//             : PlayerStateType::Jump);
//
//         return;
//     }
//
//     // 入力は方向と時間の判定に使い、カウント中も横移動は加えない。
//     player_.StopMove();
//     if (pushingWall)
//     {
//         const Vec3 direction = input.moveDir.Normalized();
//         // 約60度以内の違いは同じ方向として扱う。
//
//         if (Vec3::Dot(direction, pushDirection_) < 0.5f)
//         {
//             pushTime_ = 0.0f;
//             pushDirection_ = direction;
//         }
//         pushTime_ += deltaTime;
//     }
//     else
//     {
//         pushTime_ = 0.0f;
//         pushDirection_ = {};
//     }
//
//     // 入力中は、上端を越えないように登る速度を制限する。
//     if (player_.IsGrounded() || player_.IsNearWallTop())
//     {
//         float ClimbingSpeed = player_.GetClimbingSpeed();
//         if (pushTime_ >= Const::PLAYER_WALL_CLIMB_WAIT && deltaTime > 0.0f)
//         {
//             float MAXHeight = (std::max)(0.0f, player_.GetWallTopY() - player_.GetPosition().y);
//             ClimbingSpeed = (std::min)(player_.GetClimbingSpeed(), MAXHeight / deltaTime);
//         }
//         player_.SetVerticalSpeed(ClimbingSpeed);
//     }
//     else if (pushingWall && pushTime_ >= Const::PLAYER_WALL_CLIMB_WAIT)
//     {
//         // 壁を押し続けたら、待ち時間後に登り始める。
//         player_.SetVerticalSpeed(player_.GetClimbingSpeed());
//     }
//     DxPlus::Debug::SetFormatString(L"state:climbing");
// }

void PlayerClimbingState::Exit()
{
    player_.EndPillarElevator();
    player_.StopMove();
    player_.SetVerticalSpeed(0.0f);
    DxPlus::Debug::SetFormatString(L"PlayerClimbingState::Exit");
}
