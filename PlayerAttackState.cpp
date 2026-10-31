#include "PlayerAttackState.h"
#include "DxPlus/Debug.h"
#include "Player.h"
#include "Consts.h"
#include "DxLib.h"
#include "DxConv.h"
#include <algorithm>
#include <cmath>

PlayerAttackState::PlayerAttackState(PlayerStateMachine& stateMachine, Player& player)
	: PlayerBaseState(stateMachine, player)
{
}

void PlayerAttackState::Enter()
{
	player_.StopMove();
	//player_.SetAnimation(PlayerAnimType::Attack, false);
	
	const Vec3 forward{ std::sin(player_.GetYaw()), 0.0f, std::cos(player_.GetYaw()) };
	
	attackSphere_.center = player_.GetAttackBallSpawnPosition();
	attackSphere_.radius = Const::PLAYER_ATTACK_BALL_RADIUS;
	
	ballVelocity_ = forward * Const::PLAYER_ATTACK_BALL_SPEED;
	
	elapsed_ = 0.0f;
	hasHitTarget_ = false;
	
	ballActive_ = !player_.IsDead();

	DxPlus::Debug::SetFormatString(L"PlayerAttackState::Enter");
}

void PlayerAttackState::Tick(float deltaTime)
{
	if (player_.IsDead())
	{
		stateMachine_.ChangeState(PlayerStateType::Die);
		return;
	}
	const PlayerInput& input = player_.GetInput();
	if (input.ultimatePressed && player_.CanUseUltimate())
	{
		stateMachine_.ChangeState(PlayerStateType::UltimateAttack);
		return;
	}

	if (ballActive_)//çUåÇíÜÇ»ÇÁçXêV
	{
		const float step = (std::max)(0.0f, deltaTime);
		attackSphere_.center += ballVelocity_ * step;
		elapsed_ += step;
		ballActive_ = elapsed_ < Const::PLAYER_ATTACK_BALL_LIFETIME;
	}

	if (!ballActive_)
	{
		stateMachine_.ChangeState(input.isMoving
			? PlayerStateType::Walk
			: PlayerStateType::Idle);

		return;
	}

	DxPlus::Debug::SetFormatString(L"state:attack");
}

void PlayerAttackState::Exit()
{
	player_.StopMove();
	ballActive_ = false;
	hasHitTarget_ = false;
	attackSphere_ = {};
	ballVelocity_ = {};
	elapsed_ = 0.0f;
	DxPlus::Debug::SetFormatString(L"PlayerAttackState::Exit");
}

bool PlayerAttackState::TryRegisterHit()//çUåÇÇ™óLå¯Ç≈ÅAÇ‹ÇæÉqÉbÉgÇµÇƒÇ¢Ç»Ç¢èÍçáÇ…ÉqÉbÉgÇìoò^Ç∑ÇÈ
{
	if (!ballActive_ || hasHitTarget_) return false;

	hasHitTarget_ = true;
	return true;
}

void PlayerAttackState::Draw() const
{
#ifdef _DEBUG
	if (!ballActive_) return;

	const unsigned int color = DxLib::GetColor(0, 0, 0);
	DxLib::DrawSphere3D(
		DxConv::ToVECTOR(attackSphere_.center),
		attackSphere_.radius,
		6,
		color,
		color,
		false);
#endif
}
