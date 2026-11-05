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
#include <vector>
#include "BossAttackType.h"
#include "BossAI.h"
#include "CameraShakeType.h"

enum class BossPhase
{
	Phase1,
	Phase2
};

class BossClapSkill;
class BossSlamSkill;
class BossTailWhipSkill;
class Player;


class Boss : public ActorBase
{
public:
	static constexpr int MaxHealth = 4;

	Boss();
	~Boss() override;

	void Init() override;
	void Reset()override;
	void Update(float deltaTime)override;
	void TurnToTarget(const Vec3& targetPosition, float deltaTime);
	void Draw() const override;
	
	void SetPlayer(Player& player) { ai_.SetPlayer(player); }
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
	// 毎フレーム Boss::Update 後に呼ぶ。着弾中の初回命中だけ true。
	bool UpdateSlamAttack(const Vec3& playerPosition, const Vec3& stageCenter);
	bool IsSlamHitActive() const;
	bool IsHitByAttackSphere(const Collision::Sphere& sphere) const;
	int GetHealth() const { return health_; }
    float GetHealthProgress() const { return maxHealth_ > 0 ? static_cast<float>(health_) / maxHealth_ : 0.0f; }
	BossPhase GetPhase() const { return phase_; }
	void TakeDamage(int damage = 1);

	void SetDownGauge(float value) { downGauge_ = (std::max)(0.0f, (std::min)(maxDownGauge_, value)); }
    float GetDownGaugeProgress() const { return maxDownGauge_ > 0.0f ? downGauge_ / maxDownGauge_ : 0.0f; }
    void ResetDownGauge() { downGauge_ = maxDownGauge_; }
	void UpdateDown(float deltaTime);

	// camera

	// camera
	void CameraShakeRequest(
		CameraShakeType type,
		float duration = 0.25f,
		float strength = 20.0f,
		float frequency = 20.0f
	);

	CameraShakeType GetCameraShakeRequest() const;
	float GetCameraShakeDuration() const { return cameraShakeDuration_; }
	float GetCameraShakeStrength() const { return cameraShakeStrength_; }
	float GetCameraShakeFrequency() const { return cameraShakeFrequency_; }

	void ResetCameraShakeRequest();

#ifdef _DEBUG
	void DrawClapAttackDebug() const;
	void DrawSlamAttackDebug() const;
	bool& IsAIActive() { return activeAI; }
#endif

	AnimationHandle GetAnimationHandle(BossAnimType::Type type) const;
	ModelAnimation& GetAnimation() { return animation_; }
	void UpdateAnimation(float deltaTime);

	// Skill
	bool UseSkill(BossAttackType type);
	bool IsTailWhipActive() const;
	bool IsTailWhipHitActive() const;
	Collision::Capsule GetTailWhipHitCapsule(const Vec3& stageCenter, float stageRadius, float attackHeight) const;
private:
	bool IsSlamSkillActive() const;

	ModelObject model_{};
	Vec3 velocity_{};
	IAttackSkill* activeSkill_{ nullptr };
	BossStateMachine stateMachine_;


	BossPhase phase_{ BossPhase::Phase1 };

	int health_{ MaxHealth };
	int maxHealth_{ MaxHealth };
	float downGauge_{ 0.0f };
	float maxDownGauge_{ 100.0f };
    bool damageTakenThisFrame_{};
	float attackPower_{ 10.0f };

	CameraShakeType cameraShakeRequest_{ CameraShakeType::None };
	float cameraShakeDuration_{ 0.25f };
	float cameraShakeStrength_{ 20.0f };
	float cameraShakeFrequency_{ 20.0f };

	// アニメーション関連
	ModelAnimation animation_;

	BossAnimType::Type currentAnimType_{ BossAnimType::Idle };
	bool currentAnimLoop_{ true };
	AnimationHandle idleAnimHandle_{};
	AnimationHandle tailWhipAnimHandle_{};
	AnimationHandle downAnimHandle_{};
	AnimationHandle atkHitAnimHnadle_{};
	AnimationHandle dieAnimHandle_{};
	AnimationHandle ClapAnimHandle_{};

	// Skill
	std::unique_ptr<BossClapSkill> clapSkill_;
	std::unique_ptr<BossSlamSkill> slamSkill_;
	std::unique_ptr<BossTailWhipSkill> tailWhipSkill_;

	BossAI ai_; // AI
#ifndef NDEBUG
	bool activeAI{ false }; // デバッグ用。falseにするとAIが無効化される。
#endif

};
