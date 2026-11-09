#include "PlayerAttackState.h"
#include "DxPlus/Debug.h"
#include "Player.h"
#include "Consts.h"
#include "DxLib.h"
#include "DxConv.h"
#include <algorithm>
#include "SoundManager.h"
#include "ResourceManager.h"

PlayerAttackState::PlayerAttackState(PlayerStateMachine& stateMachine, Player& player)
	: PlayerBaseState(stateMachine, player)
{
}

void PlayerAttackState::Enter()
{
	//player_.StopMove();
	player_.SetAnimation(PlayerAnimType::Attack, false);
	player_.GetAnimation().Start();
	player_.SetMoveRate(0.7f);
	
	attackSphere_.center = player_.GetAttackBallSpawnPosition();
	attackSphere_.radius = Const::PLAYER_ATTACK_BALL_RADIUS;
	
	elapsed_ = 0.0f;
	hasHitTarget_ = false;
	stateMachine_.NotifyEvent(PlayerActionEvent::Attacked);
	ballActive_ = !player_.IsDead();

	SoundManager::PlaySE(
		RM().GetSound(
			ResourceKeys::Sound_PlayerDamage
		)
	);

	DxPlus::Debug::SetFormatString(L"PlayerAttackState::Enter");
}

void PlayerAttackState::Tick(float deltaTime)
{
	if (player_.IsDead())
	{
		stateMachine_.ChangeState(PlayerStateType::Die);
		return;
	}
	player_.UpdateAnimation(deltaTime);
	const PlayerInput& input = player_.GetInput();
	if (input.ultimatePressed && player_.CanUseUltimate())
	{
		stateMachine_.ChangeState(PlayerStateType::UltimateAttack);
		return;
	}

	player_.SetMoveDirection(input.moveDir);

	if (ballActive_)//攻撃中なら更新
	{
		const float step = (std::max)(0.0f, deltaTime);
		elapsed_ += step;
		ballActive_ = elapsed_ < Const::PLAYER_ATTACK_BALL_LIFETIME;
	}

	if (!ballActive_ && player_.GetAnimation().IsFinished())
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
	player_.SetMoveRate(1.0f);
	ballActive_ = false;
	hasHitTarget_ = false;
	attackSphere_ = {};
	elapsed_ = 0.0f;
	DxPlus::Debug::SetFormatString(L"PlayerAttackState::Exit");
}

bool PlayerAttackState::TryRegisterHit()//攻撃が有効で、まだヒットしていない場合にヒットを登録する
{
	if (!ballActive_ || hasHitTarget_) return false;

	hasHitTarget_ = true;
	return true;
}

void PlayerAttackState::Draw() const
{
#ifdef _DEBUG
	if (!ballActive_) return;

	const unsigned int color = hasHitTarget_
		? DxLib::GetColor(80, 255, 100)
		: DxLib::GetColor(255, 230, 80);
	DxLib::DrawSphere3D(
		DxConv::ToVECTOR(attackSphere_.center),
		attackSphere_.radius,
		6,
		color,
		color,
		false);
#endif
}
