#include "Player.h"
#include "TransformDefaults.h"

#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "Consts.h"
#include "Raycast.h"
#include "DxLib.h"
#include "Stage.h"

#include <cmath>
#include "Sound3D.h"


Player::Player()
	: stateMachine_(*this)
{
}


void Player::Init()
{
	SetTransform(TransformDefaults::GamePlayer);

    const int modelHandle = RM().GetModel(ResourceKeys::Model_Paladin);
	if (modelHandle < 0) return;

	model_.modelKey = ResourceKeys::Model_Paladin;
}


void Player::Reset()
{
	SetTransform(TransformDefaults::GamePlayer);


	moveInput_ = {};
	velocity_ = { 0.0f, 0.0f, 0.0f };

	// 空中から開始し、ステージへのレイキャストで接地させる。
	isGrounded_ = false;
	animation_.Reset();
	SetAnimation(PlayerAnimType::Idle, true);

    stateMachine_.Initialize();
    // 更新を止めても描画できるように初期ポーズを用意する
    animation_.Play3D(RM().GetModel(model_.modelKey), currentAnimIndex_, currentAnimLoop_, Const::ANIM_FPS);
    animation_.Update(0.0f);
}


void Player::Update(float deltaTime)
{
	input_.Update();

	if (input_.jumpPressed)
	{
		JumpAction();
	}

	stateMachine_.Tick(deltaTime);

	// 中心がどっちにあるか調べる。高さはここでは考えない
	Vec3 forward = movementCenter_ - position_;
	forward.y = 0.0f;
	const float radius = forward.Length();
	const Vec3 input = moveInput_.Normalized();

	bool canMove = true;
	if (radius < 0.001f) 
	{
		canMove = false;
	}
	if (input.x == 0.0f && input.z == 0.0f) // 何も押していない
	{
		canMove = false;
	}


	if (canMove)
	{
		forward = forward.Normalized();

		// 中心への向きを90度回すと、右向きになる
		const Vec3 right{ forward.z, 0.0f, -forward.x };

		Vec3 moveDir{};
		const float moveDis = Const::PLAYER_MOVE_SPEED * deltaTime;
		float nextRadius = radius;

		if (input.z > 0.0f) // Wkey
		{
			const float forwardInput = input.z;
			moveDir += forward * forwardInput;
			nextRadius -= forwardInput * moveDis;
		}

		if (input.z < 0.0f) // Skey
		{
			const float backwardInput = -input.z;
			moveDir -= forward * backwardInput;
			nextRadius += backwardInput * moveDis;
		}

		// 中心を通り越さないように、距離は1までにする
		if (nextRadius < 1.0f)
		{
			nextRadius = 1.0f;
		}

		// W/Sで近づいた分、離れた分だけ動かす
		const float forwardDistance = radius - nextRadius;
		Vec3 nextPosition = position_ + forward * forwardDistance;

		if (input.x > 0.0f) // Dkey moveto right
		{
			const float rightInput = input.x;
			moveDir += right * rightInput;
			nextPosition += right * (rightInput * moveDis);
		}

		if (input.x < 0.0f) // Akey moveto left
		{
			const float leftI = -input.x;
			moveDir -= right * leftI;
			nextPosition -= right * (leftI * moveDis);
		}

		// 横に動くだけだと外側にずれるので、中心からの距離を合わせ直す
		Vec3 fromCenter = nextPosition - movementCenter_;
		fromCenter.y = 0.0f;
		fromCenter = fromCenter.Normalized();
		position_.x = movementCenter_.x + fromCenter.x * nextRadius;
		position_.z = movementCenter_.z + fromCenter.z * nextRadius;

		// 猫の顔も、進む方向に向ける
		TurnTowards(moveDir, deltaTime);
	}

	// 立ち止まっていても、重力やジャンプは動かす
	if (gravityEnabled_)
	{
		velocity_.y -= Const::GRAVITY * deltaTime;
	}
	position_.y += velocity_.y * deltaTime;
	isGrounded_ = false;

	SetPosition(position_);
	model_.position = GetPosition();
	model_.rotation.y = GetYaw();
	model_.rotation.x = GetPitch();

	const int modelHandle = RM().GetModel(model_.modelKey);
	animation_.Play3D(modelHandle, currentAnimIndex_, currentAnimLoop_, Const::ANIM_FPS);
	animation_.Update(deltaTime);
}

void Player::Update(float deltaTime, const Stage& stage, const Vec3& movementCenter)
{
	movementCenter_ = movementCenter;
	const float previousY = GetPosition().y;

	Update(deltaTime);

	// 落下中のみ床面に合わせ、このフレームの落下距離もレイの長さに含める。
	if (velocity_.y <= 0.0f && stage.GetModelHandle() >= 0)
	{
		Physics::RayHit hit{};
		constexpr float groundSnapDistance = 30.0f;
		const float fallDistance = previousY - position_.y;
		const float rayDistance =
			fallDistance + groundSnapDistance + Const::PLAYER_SKIN;

		if (Physics::RaycastDown(
			stage.GetModelHandle(),
			Vec3(
				GetPosition().x,
				previousY + Const::PLAYER_SKIN,
				GetPosition().z
			),
			rayDistance,
			hit))
		{
			if (position_.y - hit.point.y <= groundSnapDistance)
			{
				position_.y = hit.point.y;
				velocity_.y = 0.0f;
				isGrounded_ = true;
			}
		}
	}

	SetPosition(position_);
	model_.position = GetPosition();
	model_.rotation.y = GetYaw();
	model_.rotation.x = GetPitch();

	Sound3D::SetListener(
		GetPosition(),
		Vec3(
			std::sin(GetYaw()),
			0.0f,
			std::cos(GetYaw())
		)
	);
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


void Player::SetMoveDirection(const Vec3& moveDir)
{
	// 押している方向を覚えておく。実際に動かすのはUpdate
	moveInput_ = { moveDir.x, 0.0f, moveDir.z };
}


void Player::StopMove()
{
	moveInput_ = {};
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
   if (!gravityEnabled_ || !isGrounded_)
	{
		return;
	}

   velocity_.y =
		Const::PLAYER_JUMP_SPEED;

	isGrounded_ = false;
}


void Player::SetGravityEnabled(bool enabled)
{
	gravityEnabled_ = enabled;
	if (!gravityEnabled_)
	{
		velocity_.y = 0.0f;
	}
}
