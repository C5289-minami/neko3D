#include "Boss.h"
#include "Boss.h"
#include "Consts.h"
#include "TransformDefaults.h"
#include "ResourceKeys.h"
#include "ResourceManager.h"

#include <cmath>
#include "DxPlus/Debug.h"


Boss::Boss()
	: stateMachine_(*this)
{
	model_.modelKey = ResourceKeys::Model_Boss;
	currentAnimIndex_ = 0;
}

void Boss::Init()
{
	activeSkill_ = nullptr;
	stateMachine_.Initialize();
}

void Boss::Reset()
{
    model_.collisionHighlighted = false;
    SetTransform(TransformDefaults::GameBoss);
	velocity_ = {};
	activeSkill_ = nullptr;
 SetAnimation(BossAnimType::Idle, true);
    animation_.Reset();
    stateMachine_.Initialize();
    // 更新を止めても描画できるように初期ポーズを用意する
    animation_.Play3D(RM().GetModel(model_.modelKey), currentAnimIndex_, currentAnimLoop_, Const::ANIM_FPS);
    animation_.Update(0.0f);
}

void Boss::Update(float deltaTime)
{
	
	stateMachine_.Tick(deltaTime);
	model_.position += velocity_ * deltaTime;

	const int modelHandle = RM().GetModel(model_.modelKey);
 animation_.Play3D(modelHandle, currentAnimIndex_, currentAnimLoop_, Const::ANIM_FPS);
	animation_.Update(deltaTime);

	DxPlus::Debug::SetString(L"Kで歩き / スキルはBoss::UseSkillから発動");
}

void Boss::Draw() const
{
    model_.ApplyCollisionColor();
	animation_.Draw3D(model_.position, model_.scale, model_.rotation);
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

const int Boss::GetModelHandle() const { return RM().GetModel(model_.modelKey); }

float Boss::GetAnimTotalTime(BossAnimType::Type type) const 
{ 
	float totalTime{};
	float totalFlame{};
	int index = static_cast<int>(type);
	int modelHandle = RM().GetModel(model_.modelKey);

	totalFlame = DxLib::MV1GetAnimTotalTime(modelHandle, index);
	totalTime = totalFlame / Const::ANIM_FPS; // 秒数に変換
	return totalTime;
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
