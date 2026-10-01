#include "Boss.h"
#include "ResourceKeys.h"

#include <cmath>


Boss::Boss()
    : stateMachine_(*this)
{
    model_.modelKey = ResourceKeys::Model_Paladin;
}

void Boss::Init()
{
    stateMachine_.Initialize();
}

void Boss::Reset()
{
    model_.position = {0.0f,-1000.0f,0.0f};
	float scaleFactor = 1500.0f; // スケールの倍率を指定
	model_.scale = { scaleFactor, scaleFactor, scaleFactor };
    model_.rotation = {};
    velocity_ = {};
    stateMachine_.Initialize();
}

void Boss::Update(float deltaTime)
{
    stateMachine_.Tick(deltaTime);
    model_.position += velocity_ * deltaTime;
}

void Boss::Draw() const
{
    model_.Draw();
}

void Boss::SetMoveDirection(const Vec3& moveDirection, float speed)
{
    Vec3 direction{ moveDirection.x, 0.0f, moveDirection.z };
    if (direction.LengthSq() <= 0.0f || speed <= 0.0f)
    {
        StopMove();
        return;
    }

    direction = direction.Normalized();
    velocity_ = direction * speed;
    model_.rotation.y = std::atan2(direction.x, direction.z);
    stateMachine_.ChangeState(BossStateType::Walk);
}

void Boss::StopMove()
{
    velocity_ = {};
    stateMachine_.ChangeState(BossStateType::Idle);
}
