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
#include "Collision.h"
#include <memory>
#include "BossAttackType.h"

enum class BossPhase
{
	Phase1,
	Phase2
};

class BossClapSkill;
class BossSlamSkill;
class BossTailWhipSkill;

class Boss : public ActorBase
{
public:
	static constexpr int MaxHealth = 4;

	Boss();
	~Boss() override;

	void Init() override;
	void Reset()override;
	void Update(float deltaTime)override;
	void Update(float deltaTime, const Vec3& targetPosition);
	void Draw() const override;

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
	void SetAnimation(
		BossAnimType::Type type,
		bool loop = true,
      bool playAfterBlend = true,
		float blendSeconds = 0.15f
	);
	float GetAnimTotalTime(BossAnimType::Type type) const;

	bool UpdateHandAttack(float deltaTime, const Vec3& playerPosition, const Vec3& stagePosition);
	bool IsHitByAttackSphere(const Collision::Sphere& sphere) const;
	int GetHealth() const { return health_; }
	BossPhase GetPhase() const { return phase_; }
	void TakeDamage(int damage = 1);
#ifdef _DEBUG
	void DrawClapAttackDebug() const;
#endif

	AnimationHandle GetAnimationHandle(BossAnimType::Type type) const;
	ModelAnimation& GetAnimation() { return animation_; }
	void UpdateAnimation(float deltaTime);

	// Skill
	bool UseSkill(BossAttackType type);

	bool IsTailWhipHitActive() const;
	Collision::Capsule GetTailWhipHitCapsule(const Vec3& stageCenter, float stageRadius, float attackHeight) const;
private:
	ModelObject model_{};
	Vec3 velocity_{};
	IAttackSkill* activeSkill_{ nullptr };
	BossStateMachine stateMachine_;


	BossPhase phase_{ BossPhase::Phase1 };

	int health_{ MaxHealth };
	int maxHealth_{ MaxHealth };
	float downGauge_{ 0.0f };
	float maxDownGauge_{ 100.0f };
	float attackPower_{ 10.0f };


	// アニメーション関連
	ModelAnimation animation_;

	BossAnimType::Type currentAnimType_{ BossAnimType::Idle };
	bool currentAnimLoop_{ true };
	AnimationHandle idleAnimHandle_{};
	AnimationHandle tailWhipAnimHandle_{};
	AnimationHandle damageAnimHandle_{};
	AnimationHandle atkHitAnimHnadle_{};
	AnimationHandle dieAnimHandle_{};
	AnimationHandle ClapAnimHandle_{};

	// Skill
	std::unique_ptr<BossClapSkill> clapSkill_;
	std::unique_ptr<BossSlamSkill> slamSkill_;
	std::unique_ptr<BossTailWhipSkill> tailWhipSkill_;
};
