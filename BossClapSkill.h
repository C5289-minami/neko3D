#pragma once

#include "BossAttackType.h"
#include "BossPoseSkill.h"
#include "Consts.h"
#include "Collision.h"
#include "DxLib.h"

#include <array>
#include <algorithm>

namespace BossClap
{
	// 秒数
	constexpr float AimDurationSeconds = 2.0f;

	// アニメーション時間
	constexpr float AttackStartFrame = 43.0f;
	constexpr float AttackEndFrame = 52.0f;

	// 当たり判定
	constexpr float PlayerCenterHeight = 30.0f;
	constexpr float PlayerHitRadius = 30.0f;
}


class BossClapSkill final : public BossPoseSkill
{
public:
	static constexpr BossAttackType Type = BossAttackType::Clap;

	explicit BossClapSkill(Boss& boss)
		: BossPoseSkill(boss)
	{
		SetFinishCondition(AttackStateType::PreAction, PhaseFinishCondition::AnimationFinished);
		SetFinishCondition(AttackStateType::Aiming, PhaseFinishCondition::MotionFinished);
		GetPhaseMotion(AttackStateType::Aiming).SetDuration(BossClap::AimDurationSeconds);
		SetFinishCondition(AttackStateType::Attack, PhaseFinishCondition::AnimationFinished);
		SetFinishCondition(AttackStateType::Return, PhaseFinishCondition::AnimationFinished);
	}

	void Reset()
	{
		BossPoseSkill::Reset();
		distance_ = InitialArcDistance;
		handAttackActive_ = false;
		for (AttackHand& hand : hands_)
		{
			hand.hitSphere = {};
			hand.isHit = false;
		}
		attackRadius_ = 0.0f;
		attackForward_ = {};
		attackRadiusInitialized_ = false;
	}

	void StartHandAttack()
	{
		distance_ = InitialArcDistance;
		handAttackActive_ = true;
		attackRadiusInitialized_ = false;

		const float yaw = boss_.GetYaw();
		attackForward_ = {
			-std::sin(yaw),
			0.0f,
			-std::cos(yaw)
		};

		for (AttackHand& hand : hands_)
		{
			hand.hitSphere.radius = HandHitRadius;
			hand.isHit = true;
		}
	}
	void UpdateTargetPosition(const Vec3& playerPosition)
	{
		if (handAttackActive_)
			return;

		targetPosition_.y =
			playerPosition.y + BossClap::PlayerCenterHeight;
	}

	bool UpdateHandAttack(
		const Vec3& playerPosition,
		const Vec3& stagePosition)
	{
		if (!handAttackActive_)
			return false;

		const Vec3 playerCenter =
			playerPosition + Vec3::Up() * BossClap::PlayerCenterHeight;

		const float animationTime = boss_.GetAnimation().GetCurrentTime();

		const float progress = std::clamp(
			(animationTime - BossClap::AttackStartFrame) /
			(BossClap::AttackEndFrame - BossClap::AttackStartFrame),
			0.0f,
			1.0f
		);

		distance_ = InitialArcDistance * (1.0f - progress);

		// 攻撃開始時のプレイヤー位置から軌道の半径を固定する。
		if (!attackRadiusInitialized_)
		{
			Vec3 toPlayer = playerPosition - stagePosition;
			toPlayer.y = 0.0f;

			attackRadius_ = toPlayer.Length();
			attackRadiusInitialized_ = true;
		}

		if (attackRadius_ < 1000.0f * Const::EPS)
			return false;

		const Vec3 right{
			attackForward_.z,
			0.0f,
			-attackForward_.x
		};

		const float angle = distance_ / attackRadius_;

		const Vec3 front = attackForward_ * std::cos(angle);
		const Vec3 side = right * std::sin(angle);

		hands_[0].hitSphere.center =
			stagePosition + (front - side) * attackRadius_;

		hands_[1].hitSphere.center =
			stagePosition + (front + side) * attackRadius_;

		// アニメーション終了時に攻撃判定を終了。
		if (progress >= 1.0f)
			handAttackActive_ = false;

		const Collision::Sphere playerBody{
			playerCenter,
			BossClap::PlayerHitRadius
		};

		for (AttackHand& hand : hands_)
		{
			if (!hand.isHit)
				continue;

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
	float attackRadius_{};
	Vec3 attackForward_{};
	bool attackRadiusInitialized_{};

	float distance_{ InitialArcDistance };
	bool handAttackActive_{};
	Vec3 targetPosition_{};
	std::array<AttackHand, 2> hands_{};

	void EnterPhase(AttackStateType phase) override
	{
		switch (phase)
		{
		case AttackStateType::PreAction:
			// AnimTypeをに設定し、アニメーションを再生する
			boss_.SetAnimation(BossAnimType::Clap, false, 1.0f);
			boss_.GetAnimation().PlayTo(43.0f);
			break;
		case AttackStateType::Aiming:
			// アニメーションの再生位置を設定する
			boss_.GetAnimation().PlayTo(43.0f);
			break;
		case AttackStateType::Attack:
			boss_.GetAnimation().PlayTo(BossClap::AttackEndFrame);
			boss_.StopRotation();
			StartHandAttack();
			break;
		case AttackStateType::Recovery:
			boss_.GetAnimation().Start(15.0f);
			break;
		case AttackStateType::Return:
			// アニメーションの再生を開始する
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
			boss_.CameraShakeRequest(CameraShakeType::Mixed, 0.5f, 20.0f, 20.0f);
			break;
		case AttackStateType::Recovery:
			break;
		case AttackStateType::Return:
			boss_.StartRotation();
			break;
		}
	}
};