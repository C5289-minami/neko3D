#include "Boss.h"
#include "Boss.h"
#include "ResourceKeys.h"
#include "ResourceManager.h"

#include <cmath>
#include "DxPlus/Debug.h"


Boss::Boss()
    : stateMachine_(*this)
{
    model_.modelKey = ResourceKeys::Model_Boss;
}

void Boss::Init()
{
    activeSkill_ = nullptr;
    stateMachine_.Initialize();
}

void Boss::Reset()
{
    model_.position = {0.0f,-1000.0f,0.0f};
	float scaleFactor = 1500.0f; // スケールの倍率を指定
	model_.scale = { scaleFactor, scaleFactor, scaleFactor };
    model_.rotation = {};
    velocity_ = {};
    activeSkill_ = nullptr;
    animationDraw_.Reset();
    stateMachine_.Initialize();
}

void Boss::Update(float deltaTime)
{
    stateMachine_.Tick(deltaTime);
    model_.position += velocity_ * deltaTime;

    DxPlus::Debug::SetString(L"Kで歩き / スキルはBoss::UseSkillから発動");
}

void Boss::Draw() const
{
    const int modelHandle = RM().GetModel(model_.modelKey);
    animationDraw_.DrawAnim3D(modelHandle, model_.position, 0, true, 1.0f, model_.scale, model_.rotation);
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
   if (stateMachine_.GetCurrentType() == BossStateType::Walk)
    {
        stateMachine_.ChangeState(BossStateType::Idle);
    }
}

bool Boss::UseSkill(IAttackSkill& skill)
{
    if (stateMachine_.GetCurrentType() == BossStateType::Attack)
    {
        return false;
    }

    activeSkill_ = &skill;
    stateMachine_.ChangeState(BossStateType::Attack);
   if (stateMachine_.GetCurrentType() != BossStateType::Attack)
    {
        activeSkill_ = nullptr;
        return false;
    }
    return true;
}
