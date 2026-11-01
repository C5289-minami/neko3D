#pragma once

#include "BossAttackType.h"
#include "BossPoseSkill.h"

class BossHairBallSkill final : public BossPoseSkill
{
public:
    static constexpr BossAttackType Type = BossAttackType::HairBall;

    explicit BossHairBallSkill(Boss& boss) : BossPoseSkill(boss) {}
   
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
			boss_.SetAnimation(BossAnimType::TailWhip, false, false);
			break;
		case AttackStateType::Aiming:
			break;
		case AttackStateType::Recovery:
			break;
		case AttackStateType::Return:
			break;
		case AttackStateType::Attack:
			boss_.GetAnimation().Start();
			break;
		}
	}
};