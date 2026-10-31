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
#include<vector>
#include <memory>
#include "BossAttackType.h"

enum class BossPhase
{
	Phase1,
	Phase2
};

class BossHairBallSkill;
class BossClapSkill;
class BossBiteSkill;
class BossTailWhipSkill;


struct AttackData
{
	Collision::Sphere hitSphere;
	bool isHit{ false };
};


class Boss : public ActorBase
{
public:
	static constexpr int MaxHealth = 4;

	Boss();
	~Boss() override;

	void Init() override;
	void Reset()override;
	void Update(float deltaTime)override;
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
	void SetAnimation(BossAnimType::Type type, bool loop = true)
	{
		currentAnimType_ = type;
		currentAnimLoop_ = loop;
	}
	float GetAnimTotalTime(BossAnimType::Type type) const;

	void StartHandAttack();
	bool UpdateHandAttack(float deltaTime, const Vec3& playerPosition, const Vec3& stagePosition);
	bool IsHitByAttackSphere(const Collision::Sphere& sphere) const;
	int GetHealth() const { return health_; }
	BossPhase GetPhase() const { return phase_; }
	void TakeDamage();
#ifdef _DEBUG
	void DrawHandAttackDebug() const;
#endif

	AnimationHandle GetAnimationHandle(BossAnimType::Type type) const;
	ModelAnimation& GetAnimation() { return animation_; }
	void UpdateAnimation(float deltaTime);

	// Skill
	bool UseSkill(BossAttackType type);
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

	float counter_{ 0.0f };
	float distance_{ 200.0f };
	bool handAttackActive_{ false };
	Vec3 oldPos{};
	std::vector<AttackData> attackHands_{};// 攻撃判定の情報を保持

	// アニメーション関連
	ModelAnimation animation_;

	BossAnimType::Type currentAnimType_{ BossAnimType::Idle };
	bool currentAnimLoop_{ true };
	AnimationHandle idleAnimHandle_{};
	AnimationHandle tailWhipAnimHandle_{};

	// Skill
	std::unique_ptr<BossHairBallSkill> hairBallSkill_;
	std::unique_ptr<BossClapSkill> clapSkill_;
	std::unique_ptr<BossBiteSkill> biteSkill_;
	std::unique_ptr<BossTailWhipSkill> tailWhipSkill_;
};
