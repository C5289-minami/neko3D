#pragma once

#include "BossAttackType.h"
#include "BossPoseSkill.h"

class BossTailWhipSkill final : public BossPoseSkill
{
public:
	static constexpr BossAttackType Type = BossAttackType::TailWhip;

	explicit BossTailWhipSkill(Boss& boss)
		: BossPoseSkill(boss)
	{
	}
private:
    void ApplyPhase(AttackStateType phase, float progress) override
    {
        auto& animation = boss_.GetAnimation();

        switch (phase)
        {
        case AttackStateType::PreAction:
            boss_.SetAnimation(BossAnimType::TailWhip, false, false);
            break;

        case AttackStateType::Aiming:
            animation.PlayTo(30.0f);
            break;

        case AttackStateType::Attack:
            animation.PlayTo(43.0f,3.0f);
            break;

        case AttackStateType::Recovery:
			animation.Start(15.0f);
            break;

        case AttackStateType::Return:
            boss_.SetAnimation(BossAnimType::Idle);
            break;
        }
    }

};