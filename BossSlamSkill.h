#pragma once

#include "BossAttackType.h"
#include "BossPoseSkill.h"
#include "DxLib.h"
#include "DxPlus/Debug.h"
#include "Consts.h"

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
	}

	void Reset()
	{
		BossPoseSkill::Reset();

		phase_ = AttackStateType::None;
		hitCapsules_ = {};

		targetLocked_ = false;
		hitActive_ = false;
		hitRegistered_ = false;
	}

	// プレイヤーの足元とステージ中心のワールド座標をつかう
	bool UpdateAttack(const Vec3& playerPosition, const Vec3& stageCenter)
	{
		if (!targetLocked_ && (phase_ == AttackStateType::PreAction ||
			phase_ == AttackStateType::Aiming || phase_ == AttackStateType::Attack))
		{
			hitCapsules_ = {};

			//中心からプレイヤーまでの距離と向きを求める
			Vec3 playerFromCenter = playerPosition - stageCenter;
			playerFromCenter.y = 0.0f;

			const float orbitRadius = playerFromCenter.Length();
			if (orbitRadius > Const::EPS)
			{
				//プレイヤーを向く角度を得る。
				const float playerAngle = std::atan2(playerFromCenter.z, playerFromCenter.x);

				const float halfSpacingAngle = (std::min)(CapsuleArcSpacing * 0.5f / orbitRadius, 1.0f);
				for (int i = 0; i < hitCapsules_.size(); ++i)
				{
					//二個挟むから一個置いた反対側へ
					const float sideDirection = (i == 0) ? -1.0f : 1.0f;
					const float capsuleAngle = playerAngle + halfSpacingAngle * sideDirection;

					// プレイヤーと同じ円周半径を保ち、足元の高さにカプセル中心を置く。
					const Vec3 capsuleCenter{
						stageCenter.x + std::cos(capsuleAngle) * orbitRadius,
						playerPosition.y,
						stageCenter.z + std::sin(capsuleAngle) * orbitRadius
					};

					// カプセルの長い向きを円周に沿わせる。
					const Vec3 alongStage{
						-std::sin(capsuleAngle) * HitHalfLength,
						0.0f,
						 std::cos(capsuleAngle) * HitHalfLength
					};
					hitCapsules_[i] = { capsuleCenter - alongStage, capsuleCenter + alongStage, HitRadius };
				}
			}
			// 振り下ろし開始時に位置と向きを固定する。
			targetLocked_ = phase_ == AttackStateType::Attack;
		}

		if (!IsHitActive() || hitRegistered_)
			return false;

		const Collision::Sphere playerBody{
			playerPosition + Vec3::Up() * (Const::PLAYER_HEIGHT * 0.5f), Const::PLAYER_RADIUS };
		for (const auto& capsule : hitCapsules_)
		{
			if (capsule.radius > 0.0f && Collision::IsHitCapsuleSphere(capsule, playerBody))
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
			? GetPhaseMotion(AttackStateType::Aiming).GetDuration() - GetPhaseMotion(AttackStateType::Aiming).GetElapsedTime() : 0.0f;
		const float hitRemaining = IsHitActive()
			? (std::max)(0.0f, HitDuration - GetPhaseMotion(AttackStateType::Recovery).GetElapsedTime()) : 0.0f;
		DxPlus::Debug::SetFormatString(L"ダイパン　照準残り %.2f秒 ／ %s ／ 判定残り %.2f秒",
			aimRemaining, dodging ? L"振り下ろし中" : L"振り下ろし待機", hitRemaining);

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
	static constexpr float HitRadius = 50.0f*2;			// 各カプセルの半径。
	static constexpr float HitHalfLength = 2.0f;		// 各カプセルの中心線の半分の長さ。
	static constexpr float CapsuleArcSpacing = 106.0f;	// 2つの中心間の円周上の距離。値を大きくすると間隔が広がる。
	static constexpr float HitDuration = 0.12f;			//着弾後に命中判定を出す秒数。

	AttackStateType phase_{ AttackStateType::None };
	std::array<Collision::Capsule, 2> hitCapsules_{};
	bool targetLocked_{};
	bool hitActive_{};
	bool hitRegistered_{};


	void EnterPhase(AttackStateType phase) override
	{
		phase_ = phase;

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
			hitActive_ = true;
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
			// 着弾後の有効時間を過ぎたら、命中判定を止める。
			hitActive_ = GetPhaseMotion(phase).GetElapsedTime() < HitDuration;
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
			// 硬直の終了や攻撃の中断で、命中判定を閉じる。
			hitActive_ = false;
			break;
		case AttackStateType::Return:
			break;
		}
	}
};
