#pragma once

#include "BossAttackType.h"
#include "BossPoseSkill.h"

class BossBangSkill final : public BossPoseSkill
{
public:
    static constexpr BossAttackType Type = BossAttackType::Bang;

    explicit BossBangSkill(Boss& boss) : BossPoseSkill(boss) {}

private:
    void ApplyPhase(AttackStateType phase, float progress) override
    {
        switch (phase)
        {
        case AttackStateType::PreAction:
            boss_.SetAnimation(BossAnimType::Idle);
            break;

        case AttackStateType::Aiming:
            boss_.SetAnimation(BossAnimType::Idle);
            break;

        case AttackStateType::Attack:
            boss_.SetAnimation(BossAnimType::TailWhip, false);
            break;

        case AttackStateType::Recovery:
        case AttackStateType::Return:
            boss_.SetAnimation(BossAnimType::Idle);
            break;
        }
    }
};
