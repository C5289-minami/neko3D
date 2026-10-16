#include "PlayerLungingJumpState.h"
#include "DxPlus/Debug.h"
#include "Player.h"

PlayerLungingJumpState::PlayerLungingJumpState(PlayerStateMachine& stateMachine, Player& player)
    : PlayerBaseState(stateMachine, player)
{
}

void PlayerLungingJumpState::Enter()
{
    player_.StopMove();
    player_.SetAnimation(PlayerAnimType::Lunging_Jump, false);
    player_.StartLungingJump(player_.GetInput().moveDir);
    DxPlus::Debug::SetFormatString(L"PlayerLungingJumpState::Enter");
}

void PlayerLungingJumpState::Tick(float)
{
    if (player_.IsGrounded() || !player_.IsGravityEnabled())
    {
        stateMachine_.ChangeState(player_.GetInput().isMoving
            ? PlayerStateType::Walk : PlayerStateType::Idle);
        return;
    }

    // 空中でも入力を更新し、通常移動と同じ回り込み処理を使う。
    player_.SetMoveDirection(player_.GetInput().moveDir);
    DxPlus::Debug::SetFormatString(L"state:lunging_jump");
}

void PlayerLungingJumpState::Exit()
{
    // 着地したら突進を止める
    player_.StopMove();
    DxPlus::Debug::SetFormatString(L"PlayerLungingJumpState::Exit");
}
