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

    // ジャンプに成功した場合だけ通知する
    if (player_.JumpAction())
    {
        stateMachine_.NotifyEvent(PlayerActionEvent::Jumped);
    }

    DxPlus::Debug::SetFormatString(L"PlayerJumpState::Enter");
}

void PlayerJumpState::Tick(float)
{
    if (player_.IsGrounded() || !player_.IsGravityEnabled())
    {
        stateMachine_.ChangeState(player_.GetInput().isMoving
            ? PlayerStateType::Walk : PlayerStateType::Idle);
        return;
    }

    const PlayerInput& input = player_.GetInput();

    // ここは「通常ジャンプ中」だけ実行される。
    // jumpPressedは押した瞬間だけtrueなので、長押しで2回目が出ることはない。
    // Spaceを一度離して、空中でもう一度押したら、ダブルジャンプへ切り替える。
    if (input.jumpPressed && player_.CanDoubleJump())
    {
        stateMachine_.ChangeState(PlayerStateType::DoubleJump);
        // 切り替えた後は、通常ジャンプ側の処理を続けない。
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
