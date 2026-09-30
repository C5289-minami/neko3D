#include "Player.h"

#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "DxConv.h"
#include "Consts.h"
#include "DxLib.h"

#include <cmath>

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
	SetPosition(Vec3(0.0f, 0.0f, 0.0f));
	SetYaw(0.0f);

	velocity_ = Vec3(0.0f, 0.0f, 0.0f);
	scale_ = Vec3(100.0f, 100.0f, 100.0f);
	isGrounded_ = true;

	if (modelHandle >= 0)
	{
		DxLib::MV1SetScale(modelHandle, DxConv::ToVECTOR(scale_));
	}

	stateMachine_.Initialize();
}

void Player::Update(float deltaTime)
{
	input_.Update();
	TurnTowards(input_.moveDir, deltaTime);

	stateMachine_.Tick(deltaTime);

	Vec3 position = GetPosition();
	position.x += velocity_.x * deltaTime;
	position.z += velocity_.z * deltaTime;
	SetPosition(position);
}

void Player::Update(float deltaTime, Stage&)
{
	Update(deltaTime);
}

void Player::Draw() const
{
	if (modelHandle >= 0)
	{
		DxLib::MV1SetPosition(modelHandle, DxConv::ToVECTOR(GetPosition()));
		DxLib::MV1SetRotationXYZ(
			modelHandle,
			DxConv::ToVECTOR(Vec3(0.0f, GetYaw(), 0.0f)));
		DxLib::MV1DrawModel(modelHandle);
	}
}

void Player::SetMoveDirection(const Vec3& moveDir, float speed)
{
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
	SetYaw(GetYaw() + direction * speed * deltaTime);
}
// 入力方向から角度を求め、プレイヤーの向きを滑らかに
void Player::TurnTowards(const Vec3& moveDir, float deltaTime)
{
	if (moveDir.x == 0.0f && moveDir.z == 0.0f) return;//キー入力がなかったら
	if (deltaTime <= 0.0f) return;

	const float pi = DX_PI_F;
	const float targetAngle = std::atan2(moveDir.x, moveDir.z);
	float diff = targetAngle - GetYaw();// 目標角度との差
	//近いほうに回る
	if (diff > pi) diff -= pi * 2.0f;
	if (diff < -pi) diff += pi * 2.0f;

	float speed = Const::PLAYER_TURN_RATE_90;
	const float angle = std::abs(diff);
	// 90度を超えたら、角度が大きいほど速くする
	if (angle > pi * 0.5f)
	{
		const float t = (angle - pi * 0.5f) / (pi * 0.5f);
		speed += (Const::PLAYER_TURN_RATE_180 - speed) * t;
	}

	float rate = speed * deltaTime;
	if (rate > 1.0f) rate = 1.0f;

	float yaw = GetYaw() + diff * rate;

	if (yaw > pi) yaw -= pi * 2.0f;
	if (yaw < -pi) yaw += pi * 2.0f;

	SetYaw(yaw);
}
