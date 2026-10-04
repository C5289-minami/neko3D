#pragma once
#pragma once

#include "AnimationDraw.h"
#include "BossStateMachine.h"
#include "DrawableObject.h"
#include "IAttackSkill.h"
#include "Vector3.h"
#include "BossAnimType.h"

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
	const Vec3& GetPosition() const { return model_.position; }
	void SetPosition(const Vec3& position) { model_.position = position; }
	BossStateType GetCurrentState() const { return stateMachine_.GetCurrentType(); }
	ModelObject& GetModelObject() { return model_; }
	const int GetModelHandle() const;
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
};
