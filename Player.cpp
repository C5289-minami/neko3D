#include "Player.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "DxConv.h"
#include "Consts.h"
#include "Raycast.h"

Player::Player()
	: stateMachine_(*this)
{
}

void Player::Init()
{
	modelHandle = RM().GetModel(ResourceKeys::Model_Paladin);
	if (modelHandle < 0) return;
	DxLib::MV1SetScale(modelHandle, DxConv::ToVECTOR(scale_));
}

void Player::Reset()
{
	position_ = Vec3(0.0f, 0.0f, 0.0f);
	velocity_ = Vec3(0.0f, 0.0f, 0.0f);
	scale_ = Vec3(100.0f, 100.0f, 100.0f);
	DxLib::MV1SetScale(modelHandle, DxConv::ToVECTOR(scale_));
	yaw_ = 0.0f;
	isGrounded_ = true;

	stateMachine_.Initialize();
}

void Player::Update(float deltaTime, const Stage& stage)
{
	input_.Update();
	TurnTowards(input_.moveDir, deltaTime);

	if (input_.jumpPressed)
	{
		JumpAction();
	}

	stateMachine_.Tick(deltaTime);

	position_.x += velocity_.x * deltaTime;
	position_.z += velocity_.z * deltaTime;

	if (!isGrounded_)
	{
		float oldY = position_.y;
		velocity_.y -= Const::GRAVITY * deltaTime;
		position_.y += velocity_.y * deltaTime;

		// 落下中にStageに当たったら着地
		if (velocity_.y <= 0.0f && stage.GetModelHandle() >= 0)
		{
			Physics::RayHit hit;
			float distance = oldY - position_.y + Const::PLAYER_SKIN * 2.0f;
			if (Physics::RaycastDown//stageのモデルに対してRaycastDownを行い、着地判定
			(
				stage.GetModelHandle(),
				Vec3(position_.x, oldY + Const::PLAYER_SKIN, position_.z),
				distance,
				hit))
			{
				position_.y = hit.point.y;
				velocity_.y = 0.0f;
				isGrounded_ = true;
			}
		}
	}
}void Player::Draw() const
{
	if (modelHandle >= 0)
	{
		DxLib::MV1SetPosition(modelHandle, DxConv::ToVECTOR(position_));
		DxLib::MV1SetRotationXYZ(modelHandle, DxConv::ToVECTOR(Vec3(0.0f, yaw_, 0.0f)));
		DxLib::MV1DrawModel(modelHandle);
	}
}

void Player::SetMoveDirection(const Vec3& moveDir, float speed)
{
	// 入力の方向に移動
	Vec3 move{ moveDir.x, 0.0f, moveDir.z };
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
	yaw_ += direction * speed * deltaTime;
}

// 入力方向から角度を求め、プレイヤーの向きを滑らかに
void Player::TurnTowards(const Vec3& moveDir, float deltaTime)
{
	if (moveDir.x == 0.0f && moveDir.z == 0.0f) return;//キー入力なかったら
	if (deltaTime <= 0.0f) return;

	float pi = DX_PI_F;
	float targetAngle = std::atan2(moveDir.x, moveDir.z);
	float diff = targetAngle - yaw_;// 目標角度との差

	// 近い方に回る
	if (diff > pi) diff -= pi * 2.0f;
	if (diff < -pi) diff += pi * 2.0f;

	float speed = Const::PLAYER_TURN_RATE_90;
	float angle = std::abs(diff);

	// 90度を超えたら、角度が大きいほど速くする
	if (angle > pi * 0.5f)
	{
		float t = (angle - pi * 0.5f) / (pi * 0.5f);
		speed += (Const::PLAYER_TURN_RATE_180 - speed) * t;
	}

	float rate = speed * deltaTime;
	if (rate > 1.0f) rate = 1.0f;

	yaw_ += diff * rate;//角度を補助して回転

	if (yaw_ > pi) yaw_ -= pi * 2.0f;
	if (yaw_ < -pi) yaw_ += pi * 2.0f;
}

void Player::JumpAction()
{
	if (!isGrounded_) return;

	velocity_.y = Const::PLAYER_JUMP_SPEED;
	isGrounded_ = false;
}
