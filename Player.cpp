#include "Player.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "DxConv.h"

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
	// 入力の更新
	input_.Update();

	stateMachine_.Tick(deltaTime);

	position_.x += velocity_.x * deltaTime;
	position_.z += velocity_.z * deltaTime;
}

void Player::Draw() const
{
	if (modelHandle >= 0)
	{
		DxLib::MV1SetPosition(modelHandle, DxConv::ToVECTOR(position_));
		DxLib::MV1SetRotationXYZ(modelHandle, DxConv::ToVECTOR(Vec3(0.0f, yaw_, 0.0f)));
		DxLib::MV1DrawModel(modelHandle);
	}
}

void Player::SetMoveDirection(const Vec3& localDir, float speed)
{
	// 前方向
	Vec3 forward{
		std::sin(yaw_),
		0.0f,
		std::cos(yaw_)
	};

	// 右方向
	Vec3 right = 
		Vec3::Cross(Vec3(0.0f,1.0f,0.0f),forward).Normalized();

	// 移動方向
	Vec3 move =
		forward * localDir.z +
			right * localDir.x;

	if (move .LengthSq() > 0.0f)
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
