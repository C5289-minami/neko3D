#include "PlayerDoubleJumpState.h"
#include "DxPlus/Debug.h"
#include "Player.h"
#include "EffectManager.h"
#include "SoundManager.h"
#include "ResourceManager.h"

namespace
{
    constexpr float DoubleJumpStartAnimFrame = 5.0f;
}

PlayerDoubleJumpState::PlayerDoubleJumpState(
    PlayerStateMachine& stateMachine,
    Player& player)
    : PlayerBaseState(stateMachine, player)
{
}

void PlayerDoubleJumpState::Enter()
{
    player_.StopMove();

    // ブレンドなし、自動再生なしで即時切り替え。
    player_.SetAnimation(
        PlayerAnimType::Double_Jump,
        false,
        false,
        0.0f
    );

    // 指定フレームから最後まで再生。
    player_.GetAnimation().SetTime(
        DoubleJumpStartAnimFrame
    );

    player_.GetAnimation().Start(
        Const::ANIM_FPS
    );

    player_.StartDoubleJump();
    stateMachine_.NotifyEvent(PlayerActionEvent::DoubleJumped);

    DxPlus::Debug::SetFormatString(
        L"PlayerDoubleJumpState::Enter"
    );

    SoundManager::PlaySE(RM().GetSound(ResourceKeys::Sound_Blink));
    EffectManager::PlayEffect(
        RM().GetEffect(ResourceKeys::Effect_Blink),
        player_.GetWorldPosition() + Vec3::Up() * (Const::PLAYER_HEIGHT * 0.5f),
        player_.GetWorldRotation().y);
}

void PlayerDoubleJumpState::Tick(float deltaTime)
{
    player_.UpdateAnimation(deltaTime);

    if (player_.IsGrounded() || !player_.IsGravityEnabled())
    {
        if (player_.GetInput().isMoving)
            stateMachine_.ChangeState(PlayerStateType::Walk);
        else
            stateMachine_.ChangeState(PlayerStateType::Idle);
        return;
    }

    // 空中でも方向入力を読み取り、Player側で横速度に反映する。
    player_.SetMoveDirection(player_.GetInput().moveDir);

    // Spaceを連打しても、着地するまで追加のジャンプはしない。
    DxPlus::Debug::SetFormatString(L"state:double_jump");
}

void PlayerDoubleJumpState::Exit()
{
    player_.StopMove();
    DxPlus::Debug::SetFormatString(L"PlayerDoubleJumpState::Exit");
}
