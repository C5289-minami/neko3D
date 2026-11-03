#pragma once

#include "BossAttackType.h"
#include "BossPoseSkill.h"
#include "Consts.h"
#include "Collision.h"
#include "DxLib.h"

#include <algorithm>
#include <array>
#include <cmath>

class BossSlamSkill final : public BossPoseSkill
{
public:
	static constexpr BossAttackType Type = BossAttackType::Slam;

	explicit BossSlamSkill(Boss& boss) : BossPoseSkill(boss)
	{
		SetFinishCondition(AttackStateType::PreAction, PhaseFinishCondition::AnimationFinished);
		SetFinishCondition(AttackStateType::Aiming, PhaseFinishCondition::MotionFinished);
		SetFinishCondition(AttackStateType::Attack, PhaseFinishCondition::AnimationFinished);
		SetFinishCondition(AttackStateType::Return, PhaseFinishCondition::AnimationFinished);

		//GetPhaseMotion(AttackStateType::Aiming).SetDuration(3.0f);// 狙う時間を指定
	}

	void Reset()
	{
		BossPoseSkill::Reset();
		counter_ = 0.0f;
		distance_ = InitialArcDistance;
		handAttackActive_ = false;
		for (AttackHand& hand : hands_)
		{
			hand.hitSphere = {};
			hand.isHit = false;
		}
	}

	void StartHandAttack()
	{
		counter_ = 0.0f;
		distance_ = InitialArcDistance;
		handAttackActive_ = true;
		for (AttackHand& hand : hands_)
		{
			hand.hitSphere.radius = HandHitRadius;
			hand.isHit = true;
		}
	}

	bool UpdateHandAttack(float deltaTime, const Vec3& playerPosition, const Vec3& stagePosition)
	{
		if (!handAttackActive_)
			return false;

		if (distance_ <= 0.0f)
		{
			handAttackActive_ = false;
			return false;
		}

		const Vec3 playerCenter = playerPosition + Vec3::Up() * PlayerCenterHeight;
		if (counter_ < FollowTime)
			targetPosition_ = playerCenter;

		counter_ += deltaTime;
		if (counter_ >= FollowTime)
			distance_ = (std::max)(0.0f, distance_ - HandClosingSpeed * deltaTime);

		Vec3 outward = targetPosition_ - stagePosition;
		outward.y = 0.0f;
		const float radius = outward.Length();
		if (radius < 0.001f)
			return false;
		outward /= radius;

		const Vec3 right{ outward.z, 0.0f, -outward.x };
		const float angle = distance_ / radius;
		const Vec3 circleCenter{ stagePosition.x, targetPosition_.y, stagePosition.z };
		const Vec3 front = outward * std::cos(angle);
		const Vec3 side = right * std::sin(angle);
		hands_[0].hitSphere.center = circleCenter + (front - side) * radius;
		hands_[1].hitSphere.center = circleCenter + (front + side) * radius;

		if (counter_ < FollowTime)
			return false;

		for (AttackHand& hand : hands_)
		{
			if (!hand.isHit)
				continue;

			const Collision::Sphere playerBody{ playerCenter, PlayerHitRadius };
			if (Collision::IsHitSphereSphere(hand.hitSphere, playerBody))
			{
				for (AttackHand& hitHand : hands_)
					hitHand.isHit = false;
				return true;
			}
		}
		return false;
	}

	bool IsHitByAttackSphere(const Collision::Sphere& sphere) const
	{
		if (!handAttackActive_)
			return false;

		for (const AttackHand& hand : hands_)
		{
			if (Collision::IsHitSphereSphere(hand.hitSphere, sphere))
				return true;
		}
		return false;
	}

#ifdef _DEBUG
	void DrawDebug() const
	{
		if (!handAttackActive_)
			return;

		DxLib::SetUseZBuffer3D(FALSE);
		DxLib::SetWriteZBuffer3D(FALSE);
		for (std::size_t i = 0; i < hands_.size(); ++i)
		{
			const unsigned int color = i == 0
				? DxLib::GetColor(80, 220, 255)
				: DxLib::GetColor(255, 120, 80);
			Collision::DrawDebugSphere(hands_[i].hitSphere, color);
		}
		DxLib::SetUseZBuffer3D(TRUE);
		DxLib::SetWriteZBuffer3D(TRUE);
	}
#endif
private:
	struct AttackHand
	{
		Collision::Sphere hitSphere{};
		bool isHit{};
	};

	static constexpr float InitialArcDistance = Const::BOSS_BETWEEN_HAND_ATTACK_SIDE;
	static constexpr float HandHitRadius = Const::BOSS_HAND_ATTACK_HIT_RADIUS;
	static constexpr float PlayerCenterHeight = 30.0f;
	static constexpr float PlayerHitRadius = 30.0f;
	static constexpr float FollowTime = 3.0f;
	static constexpr float HandClosingSpeed = 600.0f;

	float counter_{};
	float distance_{ InitialArcDistance };
	bool handAttackActive_{};
	Vec3 targetPosition_{};
	std::array<AttackHand, 2> hands_{};
	void EnterPhase(AttackStateType phase) override
	{
		switch (phase)
		{
		case AttackStateType::PreAction:
			// AnimTypeをSlamに設定し、アニメーションを再生する
			boss_.SetAnimation(BossAnimType::Slam, false);
			boss_.GetAnimation().PlayTo(0.0f, Const::ANIM_FPS, 3.0f);
			break;
		case AttackStateType::Aiming:
			// アニメーションの再生位置を設定する
			boss_.GetAnimation().PlayTo(0.0f);
			break;
		case AttackStateType::Attack:
			boss_.GetAnimation().PlayTo(8.0f);
			break;
		case AttackStateType::Recovery:
		case AttackStateType::Return:
			// アニメーションの再生を開始する
			boss_.GetAnimation().Start();
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
			break;
		}
	}
};