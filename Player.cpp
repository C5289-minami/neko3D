#include "Player.h"

#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "Consts.h"
#include "Raycast.h"
#include "DxLib.h"
#include "Stage.h"

#include <cmath>


Player::Player()
	: stateMachine_(*this)
{
}


void Player::Init()
{
	SetPosition({ 0.0f, 300.0f, 2300.0f });

    const int modelHandle = RM().GetModel(ResourceKeys::Model_Paladin);
	if (modelHandle < 0) return;

	model_.modelKey = ResourceKeys::Model_Paladin;
	model_.scale = Vec3(100.0f,100.0f,100.0f);
}


void Player::Reset()
{
	SetPosition({ 0.0f, 300.0f, 2300.0f });

	SetYaw(0.0f);
	SetPitch(0.0f);

	velocity_ = { 0.0f, 0.0f, 0.0f };

	isGrounded_ = true;
	animation_.Reset();
	SetAnimation(PlayerAnimType::Idle, true);

	stateMachine_.Initialize();
}


void Player::Update(float deltaTime)
{
	input_.Update();

	TurnTowards(input_.moveDir, deltaTime);

	if (input_.jumpPressed)
	{
		JumpAction();
	}

	stateMachine_.Tick(deltaTime);

	// 横方向の移動
	position_.x += velocity_.x * deltaTime;
	position_.z += velocity_.z * deltaTime;

	// 重力
	if (!isGrounded_)
	{
		velocity_.y -= Const::GRAVITY * deltaTime;
		position_.y += velocity_.y * deltaTime;
	}

	SetPosition(position_);
	model_.position = GetPosition();
	model_.rotation.y = GetYaw();
	model_.rotation.x = GetPitch();

   const int modelHandle = RM().GetModel(model_.modelKey);
	animation_.Play3D(modelHandle, currentAnimIndex_, currentAnimLoop_, Const::ANIM_FPS);
	animation_.Update(deltaTime);
}


void Player::Update(float deltaTime, const Stage& stage)
{
	const float oldY = GetPosition().y;

	Update(deltaTime);

	if (!isGrounded_ &&
		velocity_.y <= 0.0f &&
		stage.GetModelHandle() >= 0)
	{
		Physics::RayHit hit;

		const float distance =
			oldY - GetPosition().y +
			Const::PLAYER_SKIN * 2.0f;

		if (Physics::RaycastDown(
			stage.GetModelHandle(),
			Vec3(
				GetPosition().x,
				oldY + Const::PLAYER_SKIN,
				GetPosition().z
			),
			distance,
			hit))
		{
			position_.y = hit.point.y;
			velocity_.y = 0.0f;
			isGrounded_ = true;

			SetPosition(position_);
		}
	}
	model_.position = GetPosition();
	model_.rotation.y = GetYaw();
	model_.rotation.x = GetPitch();
}


void Player::Draw() const
{
	animation_.Draw3D(model_.position, model_.scale, model_.rotation);
}


void Player::SetAnimation(PlayerAnimType::Type type, bool loop)
{
	currentAnimIndex_ = static_cast<int>(type);
	currentAnimLoop_ = loop;
}


void Player::SetMoveDirection(const Vec3& moveDir, float speed)
{
	Vec3 move
	{
		moveDir.x,
		0.0f,
		moveDir.z
	};

	if (move.LengthSq() > 0.0f)
	{
		move = move.Normalized() * speed;
	}

	velocity_.x = move.x;
	velocity_.z = move.z;
}


void Player::StopMove()
{
	velocity_.x = 0.0f;
	velocity_.z = 0.0f;
}


void Player::Turn(float direction, float speed, float deltaTime)
{
	SetYaw(
		GetYaw() +
		direction * speed * deltaTime
	);
}


// 入力方向から角度を求め、
// プレイヤーの向きを滑らかに変更する
void Player::TurnTowards(
	const Vec3& moveDir,
	float deltaTime)
{
	if (moveDir.x == 0.0f &&
		moveDir.z == 0.0f)
	{
		return;
	}

	if (deltaTime <= 0.0f)
	{
		return;
	}

	const float pi = DX_PI_F;

	const float targetAngle =
		std::atan2(moveDir.x, moveDir.z);

	float diff =
		targetAngle - GetYaw();

	// 近いほうに回る
	if (diff > pi)
	{
		diff -= pi * 2.0f;
	}

	if (diff < -pi)
	{
		diff += pi * 2.0f;
	}

	float speed =
		Const::PLAYER_TURN_RATE_90;

	const float angle =
		std::abs(diff);

	// 90度を超えたら、
	// 角度が大きいほど回転速度を上げる
	if (angle > pi * 0.5f)
	{
		const float t =
			(angle - pi * 0.5f) /
			(pi * 0.5f);

		speed +=
			(Const::PLAYER_TURN_RATE_180 - speed)
			* t;
	}

	float rate =
		speed * deltaTime;

	if (rate > 1.0f)
	{
		rate = 1.0f;
	}

	float yaw =
		GetYaw() + diff * rate;

	if (yaw > pi)
	{
		yaw -= pi * 2.0f;
	}

	if (yaw < -pi)
	{
		yaw += pi * 2.0f;
	}

	SetYaw(yaw);
}


void Player::JumpAction()
{
	if (!isGrounded_)
	{
		return;
	}

	velocity_.y =
		Const::PLAYER_JUMP_SPEED;

	isGrounded_ = false;
}