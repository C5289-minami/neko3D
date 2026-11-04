#include "PlayerStateMachine.h"
#include "Player.h"
#include <utility>

PlayerStateMachine::PlayerStateMachine(Player& player)
    : player_(player),
	currentType_(PlayerStateType::Idle),
	idleState_(*this, player),
	walkState_(*this, player),
	groomingState_(*this, player),
	jumpState_(*this, player),
	doubleJumpState_(*this, player),
	attackState_(*this, player),
	ultimateAttackState_(*this, player),
	damagingState_(*this, player),
	dieState_(*this, player),
	climbingState_(*this, player),
	wallJumpState_(*this, player)
{

}
void PlayerStateMachine::Initialize()
{
	initialized_ = false;

	ChangeState(PlayerStateType::Idle);
}

void PlayerStateMachine::ChangeState(PlayerStateType type)
{
	if (type == PlayerStateType::Attack && player_.IsDead()) return;

	if (initialized_ && currentType_ == type) return;

	State* newState = FindState(type);
	if (!newState)return;

	currentType_ = type;
	initialized_ = true;
	StateMachine::SwitchState(newState);

}

bool PlayerStateMachine::IsAttackBallActive() const
{
	// 攻撃ステート中で、攻撃球が有効なら true を返す。
	return currentType_ == PlayerStateType::Attack && attackState_.IsBallActive();
}

const Collision::Sphere& PlayerStateMachine::GetAttackBallSphere() const
{
	// 攻撃ステートが持つ攻撃球の当たり判定を参照で返す。
	return attackState_.GetAttackSphere();
}

bool PlayerStateMachine::TryRegisterAttackBallHit()
{
	// 攻撃ステート中に、今回の攻撃球の命中を一度だけ登録できたか返す。
	return currentType_ == PlayerStateType::Attack && attackState_.TryRegisterHit();
}

float PlayerStateMachine::GetAvoidTime() const
{
	return currentType_ == PlayerStateType::Damaging
		? damagingState_.GetAvoidTime()
		: 0.0f;
}

bool PlayerStateMachine::TryAvoid()
{
	if (currentType_ != PlayerStateType::DoubleJump)
		return false;

	return doubleJumpState_.TryAvoid();
}

void PlayerStateMachine::Draw() const
{
	if (currentType_ == PlayerStateType::Attack)
		attackState_.Draw();
}


void PlayerStateMachine::SetEventCallback(
    std::function<void(PlayerActionEvent)> callback)
{
    eventCallback_ = std::move(callback);
}

void PlayerStateMachine::NotifyEvent(PlayerActionEvent event)
{
    if (eventCallback_)
        eventCallback_(event);
}

State* PlayerStateMachine::FindState(PlayerStateType type)
{
    switch (type)
    {
    case PlayerStateType::Idle:
        return &idleState_;
    case PlayerStateType::Walk:
        return &walkState_;
    case PlayerStateType::Grooming:
        return &groomingState_;
    case PlayerStateType::Jump:
        return &jumpState_;
    case PlayerStateType::DoubleJump:
        return &doubleJumpState_;
    case PlayerStateType::Attack:
        return &attackState_;
    case PlayerStateType::UltimateAttack:
        return &ultimateAttackState_;
    case PlayerStateType::Damaging:
        return &damagingState_;
    case PlayerStateType::Die:
        return &dieState_;
    case PlayerStateType::Climbing:
        return &climbingState_;
    case PlayerStateType::WallJump:
        return &wallJumpState_;
    default:
        return nullptr;
    }
}
