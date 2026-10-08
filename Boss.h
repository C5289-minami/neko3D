#pragma once
#pragma once

#include "AnimationDraw.h"
#include "BossStateMachine.h"
#include "DrawableObject.h"
#include "IAttackSkill.h"
#include "Vector3.h"
#include "BossAnimType.h"
#include "TransformSettings.h"

class Boss
{
public:
	Boss();

	void Init();
	void Reset();
	void Update(float deltaTime);
	void Draw() const;

	bool UseSkill(IAttackSkill& skill);
	void ClearActiveSkill() { activeSkill_ = nullptr; }

	void SetMoveDirection(const Vec3& moveDirection, float speed);
	void StopMove();
	IAttackSkill* GetActiveSkill() const { return activeSkill_; }
	ObjectTransform GetTransform() const { return { model_.scale, model_.position, model_.rotation }; }
    void SetTransform(const ObjectTransform& transform)
    {
        model_.scale = transform.scale;
        model_.position = transform.position;
        model_.rotation = transform.rotation;
    }
    const Vec3& GetPosition() const { return model_.position; }
	void SetPosition(const Vec3& position) { model_.position = position; }
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

private:
	ModelObject model_{};
	Vec3 velocity_{};
	IAttackSkill* activeSkill_{ nullptr };
	BossStateMachine stateMachine_;


	// Animation
	AnimationDraw animation_;

	int currentAnimIndex_{ -1 };
	bool currentAnimLoop_{ true };

	// �X�e�[�^�X
	float health_{ 100.0f };
	float maxHealth_{ 100.0f };
	float downGauge_{ 0.0f };
	float maxDownGauge_{ 100.0f };
	float attackPower_{ 10.0f };

	bool isHeatUp{ false };
	
};
