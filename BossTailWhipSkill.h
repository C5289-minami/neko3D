#pragma once

#include "BossAttackType.h"
#include "BossPoseSkill.h"
#include "DxPlus/DxPlusConstants.h"
#include "EffectManager.h"
#include "ResourceKeys.h"
#include "ResourceManager.h"

class BossTailWhipSkill final : public BossPoseSkill
{
public:
	static constexpr BossAttackType Type = BossAttackType::TailWhip;

	explicit BossTailWhipSkill(Boss& boss)
		: BossPoseSkill(boss)
	{
		SetFinishCondition(AttackStateType::PreAction, PhaseFinishCondition::AnimationFinished);
		SetFinishCondition(AttackStateType::Aiming, PhaseFinishCondition::AnimationFinished);
		SetFinishCondition(AttackStateType::Attack, PhaseFinishCondition::AnimationFinished);
		SetFinishCondition(AttackStateType::Return, PhaseFinishCondition::AnimationFinished);

		// 攻撃アニメーションの30～43フレームに合わせ、13フレームで一周する。
		GetPhaseMotion(AttackStateType::Attack).SetDuration(13.0f / Const::ANIM_FPS);
	}

	void Reset()
	{
		BossPoseSkill::Reset();
		hitActive_ = false;
		sweepProgress_ = 0.0f;
	}

	bool IsHitActive() const { return hitActive_; }

	Collision::Capsule GetHitCapsule(
		const Vec3& stageCenter,
		float stageRadius,
		float attackHeight) const
	{
		if (!hitActive_)
			return {};

		// DxPlus::PIを2倍すると360度になり、進捗1.0で一周する。
		constexpr float tau = 2.0f * DxPlus::PI;
		constexpr float capsuleRadius = 45.0f * 0.5f;

		const float yaw = boss_.GetYaw() - (tau * sweepProgress_);
		const Vec3 direction{ std::sin(yaw), 0.0f, std::cos(yaw) };

		const Vec3 start{ stageCenter.x, attackHeight, stageCenter.z };
		const float reach = (std::max)(0.0f, stageRadius - capsuleRadius);
		const Vec3 end = start + direction * reach;

		Collision::Capsule capsule;
		capsule.start = start;
		capsule.end = end;
		capsule.radius = capsuleRadius;
		return capsule;
	}

private:
	bool hitActive_{ false };
	float sweepProgress_{};

	void EnterPhase(AttackStateType phase) override
	{
		hitActive_ = phase == AttackStateType::Attack;
		if (hitActive_)
			sweepProgress_ = 0.0f;

		switch (phase)
		{
		case AttackStateType::PreAction:
			// AnimTypeをTailWhipに設定し、アニメーションを再生する。
			boss_.SetAnimation(BossAnimType::TailWhip, false, true);
			boss_.GetAnimation().PlayTo(0.0f, Const::ANIM_FPS, 1.0f);
			boss_.StopRotation();
			break;
		case AttackStateType::Aiming:
			// アニメーションの再生位置を設定する。
			boss_.GetAnimation().PlayTo(30.0f);
			break;
		case AttackStateType::Attack:
			// アニメーションの攻撃区間を再生する。
			boss_.GetAnimation().PlayTo(43.0f);
			EffectManager::PlayEffect(
				RM().GetEffect(ResourceKeys::Effect_BossTailWhip),
				boss_.GetWorldPosition(),
				boss_.GetWorldRotation().y + DxPlus::PI);

			// 横方向だけ強く揺らす
			boss_.CameraShakeRequest(CameraShakeType::Horizontal, 0.3f, 30.0f);
			break;
		case AttackStateType::Recovery:
		case AttackStateType::Return:
			// アニメーションの再生を開始する。
			boss_.GetAnimation().Start(15.0f);
			break;
		}
	}

	void ApplyPhase(AttackStateType phase, float progress) override
	{
		auto& animation = boss_.GetAnimation();
		switch (phase)
		{
		case AttackStateType::PreAction:
			break;
		case AttackStateType::Aiming:
			break;
		case AttackStateType::Attack:
			// フェーズの進捗を、描画と本判定で共有するカプセルの回転に使う。
			sweepProgress_ = GetPhaseMotion(AttackStateType::Attack).GetProgress();
			break;
		case AttackStateType::Recovery:
			break;
		case AttackStateType::Return:
			break;
		}
	}

	void EndPhase(AttackStateType phase) override
	{
		switch (phase)
		{
		case AttackStateType::PreAction:
			break;
		case AttackStateType::Aiming:
			break;
		case AttackStateType::Attack:
			break;
		case AttackStateType::Recovery:
			break;
		case AttackStateType::Return:
			boss_.StartRotation();
			break;
		}
	}
};