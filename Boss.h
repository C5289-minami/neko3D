#pragma once
#pragma once

#include "ModelAnimation.h"
#include "BossStateMachine.h"
#include "DrawableObject.h"
#include "IAttackSkill.h"
#include "Vector3.h"
#include "ObjectTransform.h"
#include "BossAnimType.h"
#include "ActorBase.h"
#include<vector>

struct AttackData
{
	Collision::Sphere hitSphere;
	bool isHit{ false };
};


class Boss : public ActorBase
{
public:
	Boss();

	void Init() override;
	void Reset()override;
	void Update(float deltaTime)override;
	void Draw() const override;

	bool UseSkill(IAttackSkill& skill);
	void ClearActiveSkill() { activeSkill_ = nullptr; }

	void SetMoveDirection(const Vec3& moveDirection, float speed);
	void StopMove();
	IAttackSkill* GetActiveSkill() const { return activeSkill_; }
 ObjectTransform GetTransform() const { return { model_.GetScale(), model_.GetPosition(), model_.GetRotation() }; }
    void SetTransform(const ObjectTransform& transform)
    {
        model_.SetScale(transform.scale);
		model_.SetPosition(transform.position);
		model_.SetRotation(transform.rotation);
    }
	BossStateType GetCurrentState() const { return stateMachine_.GetCurrentType(); }
	ModelObject& GetModelObject() { return model_; }
	const int GetModelHandle() const;
	const ModelObject& GetModelObject() const { return model_; }
	void SetAnimation(BossAnimType::Type type, bool loop = true)
	{
		currentAnimIndex_ = static_cast<int>(type);
		currentAnimLoop_ = loop;
	}
	float GetAnimTotalTime(BossAnimType::Type type) const;

	bool UpdateHandAttack(float deltaTime, const Vec3& playerPosition);

private:
	ModelObject model_{};
	Vec3 velocity_{};
	IAttackSkill* activeSkill_{ nullptr };
	BossStateMachine stateMachine_;


	// Animation
	ModelAnimation animation_;

	int currentAnimIndex_{ -1 };
	bool currentAnimLoop_{ true };

	float health_{ 100.0f };
	float maxHealth_{ 100.0f };
	float downGauge_{ 0.0f };
	float maxDownGauge_{ 100.0f };
	float attackPower_{ 10.0f };

	bool isHeatUp{ false };
	
	float counter_{ 0.0f };
	float distance_{ 200.0f };
	std::vector<AttackData> attackHands_{};// 攻撃判定の情報を保持


};
