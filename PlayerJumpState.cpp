#include "PlayerJumpState.h"
#include "PlayerStateMachine.h"
#include "Player.h"
#include "DxPlus/Debug.h"

PlayerJumpState::PlayerJumpState(PlayerStateMachine& stateMachine, Player& player)
    : PlayerBaseState(stateMachine, player)
{
}

void PlayerJumpState::Enter()
{
    player_.StopMove();
    player_.SetAnimation(PlayerAnimType::Jump, false);
    // 落下開始でもこの状態を使う。上向きの初速はTryJumpが設定する。
    DxPlus::Debug::SetFormatString(L"PlayerJumpState::Enter");
}

void PlayerJumpState::Tick(float)
{
    const PlayerInput& input = player_.GetInput();
    // 踏み外し直後は通常ジャンプを優先し、猶予後は未使用の二段目を使う。
    if (input.jumpPressed && player_.TryJump())
    {
        return;
    }

    if (player_.IsGrounded() || !player_.IsGravityEnabled())
    {
        stateMachine_.ChangeState(player_.GetInput().isMoving
            ? PlayerStateType::Walk : PlayerStateType::Idle);
        return;
    }

    // 2回目を押すまでは、空中でも方向キーで移動できる。
    player_.SetMoveDirection(input.moveDir);
    DxPlus::Debug::SetFormatString(L"state:jump");
}

void PlayerJumpState::Exit()
{
    player_.StopMove();
    DxPlus::Debug::SetFormatString(L"PlayerJumpState::Exit");
}
