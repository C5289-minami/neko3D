#pragma once

#include "BossAttackType.h"
#include "BossPoseSkill.h"
#include "DxLib.h"
#include "DxPlus/Debug.h"
#include "Consts.h"
#include "ResourceKeys.h"
#include "ResourceManager.h"
#include "SoundManager.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace BossSlam
{
	// 当たり判定
	constexpr float HitRadius = 160.0f;
	constexpr float HitHalfLength = 2.0f;
	constexpr float CapsuleArcSpacing = 106.0f;
	constexpr float HitDuration = 0.12f;
}

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
	}

	void Reset()
	{
		BossPoseSkill::Reset();
		phase_ = AttackStateType::None;
		hitCapsules_ = {};
		targetLocked_ = false;
		hitActive_ = false;
		hitRegistered_ = false;
		attackForward_ = {};
		attackRadius_ = 0.0f;
		attackHeight_ = 0.0f;
	}

	// プレイヤーの足元とステージ中心のワールド座標をつかう
	bool UpdateAttack(const Vec3& playerPosition, const Vec3& stageCenter)
	{
		if (!targetLocked_ &&
			(phase_ == AttackStateType::PreAction ||
				phase_ == AttackStateType::Aiming))
		{
			UpdateHitCapsules(playerPosition, stageCenter);
		}

		if (!targetLocked_ && phase_ == AttackStateType::Attack)
		{
			LockAttackPosition(playerPosition, stageCenter);
		}

		if (!IsHitActive() || hitRegistered_)
			return false;

		const Collision::Sphere playerBody{
			playerPosition + Vec3::Up() * (Const::PLAYER_HEIGHT * 0.5f),
			Const::PLAYER_RADIUS
		};

		for (const auto& capsule : hitCapsules_)
		{
			if (capsule.radius > 0.0f &&
				Collision::IsHitCapsuleSphere(capsule, playerBody))
			{
				hitRegistered_ = true;
				return true;
			}
		}

		return false;
	}

	bool IsHitActive() const
	{
		// 攻撃が着弾した直後の、有効時間内かを確認する。
		if (!hitActive_)
			return false;

		// 振り下ろし開始時に固定した判定位置があるかを確認する。
		if (!targetLocked_)
			return false;

		// それぞれのカプセルが作られているかを確認する。半径0は未配置を表す。
		if (hitCapsules_[0].radius <= 0.0f && hitCapsules_[1].radius <= 0.0f)
			return false;

		return true;
	}

	std::array<Collision::Capsule, 2> GetHitCapsules() const
	{
		if (phase_ == AttackStateType::Return || (phase_ == AttackStateType::Recovery && !hitActive_))
			return {};
		return hitCapsules_;
	}

#ifdef _DEBUG
	void DrawDebug() const
	{
		// 照準と命中判定の残り時間は、既存のフェーズの経過時間から求める。
		const bool aiming = phase_ == AttackStateType::Aiming;
		const bool dodging = phase_ == AttackStateType::Attack;
		const float aimRemaining = aiming
			? GetPhaseMotion(AttackStateType::Aiming).GetDuration() - GetPhaseMotion(AttackStateType::Aiming).GetElapsedTime()
			: 0.0f;
		const float hitRemaining = IsHitActive()
			? (std::max)(0.0f, BossSlam::HitDuration - GetPhaseMotion(AttackStateType::Recovery).GetElapsedTime())
			: 0.0f;
		DxPlus::Debug::SetFormatString(
			L"ダイパン 照準残り %.2f秒 ／ %s ／ 判定残り %.2f秒",
			aimRemaining,
			dodging ? L"振り下ろし中" : L"振り下ろし待機",
			hitRemaining);

		// 青は追従中、黄は固定後の振り下ろし中、オレンジは命中判定中。
		const unsigned int color = IsHitActive()
			? DxLib::GetColor(255, 160, 60)
			: (targetLocked_ ? DxLib::GetColor(255, 230, 80) : DxLib::GetColor(80, 220, 255));
		for (const auto& capsule : hitCapsules_)
		{
			if (capsule.radius > 0.0f)
				Collision::DrawDebugCapsule(capsule, color);
		}
	}
#endif

private:
	AttackStateType phase_{ AttackStateType::None };
	std::array<Collision::Capsule, 2> hitCapsules_{};
	bool targetLocked_{};
	bool hitActive_{};
	bool hitRegistered_{};

	Vec3 attackForward_{};
	float attackRadius_{};
	float attackHeight_{};

	void EnterPhase(AttackStateType phase) override
	{
		phase_ = phase;

		switch (phase)
		{
		case AttackStateType::PreAction:
			boss_.SetAnimation(BossAnimType::Slam, false);
			boss_.GetAnimation().PlayTo(0.0f, Const::ANIM_FPS, 3.0f);
			break;
		case AttackStateType::Aiming:
			boss_.GetAnimation().PlayTo(0.0f);
			break;
		case AttackStateType::Attack:
			boss_.GetAnimation().PlayTo(6.0f);
			boss_.StopRotation();
			break;
		case AttackStateType::Recovery:
			hitActive_ = true;
			SoundManager::PlaySE(RM().GetSound(ResourceKeys::Sound_Slam));
			break;
		case AttackStateType::Return:
			boss_.GetAnimation().Start();
			break;
		}
	}

	void ApplyPhase(AttackStateType phase, float progress) override
	{
		switch (phase)
		{
		case AttackStateType::PreAction:
		case AttackStateType::Aiming:
		case AttackStateType::Attack:
			break;
		case AttackStateType::Recovery:
			hitActive_ = GetPhaseMotion(phase).GetElapsedTime() < BossSlam::HitDuration;
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
		case AttackStateType::Aiming:
			break;
		case AttackStateType::Attack:
			boss_.CameraShakeRequest(CameraShakeType::Mixed, 0.3f, 40.0f, 40.0f);
			break;
		case AttackStateType::Recovery:
			hitActive_ = false;
			break;
		case AttackStateType::Return:
			boss_.StartRotation();
			break;
		}
	}

	void LockAttackPosition(const Vec3& playerPosition, const Vec3& stageCenter)
	{
		UpdateHitCapsules(playerPosition, stageCenter);
		targetLocked_ = true;
	}

	void UpdateHitCapsules(const Vec3& playerPosition, const Vec3& stageCenter)
	{
		hitCapsules_ = {};

		Vec3 playerFromCenter = playerPosition - stageCenter;
		playerFromCenter.y = 0.0f;

		const float orbitRadius = playerFromCenter.Length();
		if (orbitRadius <= Const::EPS)
			return;

		const float yaw = boss_.GetYaw();

		attackForward_ = {
			-std::sin(yaw),
			0.0f,
			-std::cos(yaw)
		};

		const Vec3 right{
			attackForward_.z,
			0.0f,
			-attackForward_.x
		};

		const Vec3 center{
			stageCenter.x + attackForward_.x * orbitRadius,
			stageCenter.y,
			stageCenter.z + attackForward_.z * orbitRadius
		};

		for (int i = 0; i < static_cast<int>(hitCapsules_.size()); ++i)
		{
			const float side = (i == 0) ? -1.0f : 1.0f;

			const Vec3 capsuleCenter =
				center + right * (BossSlam::CapsuleArcSpacing * 0.5f * side);

			const Vec3 along = right * BossSlam::HitHalfLength;

			hitCapsules_[i] = {
				capsuleCenter - along,
				capsuleCenter + along,
				BossSlam::HitRadius
			};
		}
	}
};