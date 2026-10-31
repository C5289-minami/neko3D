#pragma once

#include "BossAttackType.h"
#include "BossPoseSkill.h"

class BossTailWhipSkill final : public BossPoseSkill
{
public:
    static constexpr BossAttackType Type = BossAttackType::TailWhip;

    explicit BossTailWhipSkill(Boss& boss)
        : BossPoseSkill(boss, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)
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
        case AttackStateType::Aiming:
        case AttackStateType::Recovery:
        case AttackStateType::Return:
            break;

        case AttackStateType::Attack:

            boss_.SetAnimation(BossAnimType::TailWhip, false);
            break;
        }
    }
    
};