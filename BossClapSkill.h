#pragma once

#include "BossAttackType.h"
#include "BossPoseSkill.h"

#include <cmath>

class BossClapSkill final : public BossPoseSkill
{
public:
	static constexpr BossAttackType Type = BossAttackType::Clap;

	explicit BossClapSkill(Boss& boss)
		: BossPoseSkill(boss, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f)
	{
	}
	bool IsAttackFinished() const override
	{
		return boss_.GetAnimation().IsFinished();
	}
private:
	void ApplyPhase(AttackStateType phase, float progress) override
	{
		switch (phase)
		{
		case AttackStateType::PreAction:
			break;
		case AttackStateType::Aiming:
			break;
		case AttackStateType::Attack:
			boss_.SetAnimation(BossAnimType::Clap, false);
			break;
		case AttackStateType::Recovery:
			break;
		case AttackStateType::Return:
			boss_.SetAnimation(BossAnimType::Idle, true);
			break;
		}
	}
};
