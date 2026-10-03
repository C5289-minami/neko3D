#pragma once

#include "BossAttackStateType.h"
#include "BossPoseSkill.h"

#include <cmath>

class BossTailWhipSkill final : public BossPoseSkill
{
public:
    static constexpr BossAttackType Type = BossAttackType::TailWhip;

    explicit BossTailWhipSkill(Boss& boss) : BossPoseSkill(boss) {}

private:
    void ApplyPhase(AttackStateType phase, float progress) override
    {
        switch (phase)
        {
        case AttackStateType::PreAction:
        case AttackStateType::Aiming:
            ApplyPose({}, { 0.0f, -0.7f * progress, 0.0f });
            break;
        case AttackStateType::Attack:
            ApplyPose({}, { 0.0f, -0.7f + 1.4f * progress,
                0.2f * std::sin(progress * 3.14159265f) });
            break;
        case AttackStateType::Recovery:
            ApplyPose({}, { 0.0f, 0.7f * (1.0f - progress), 0.0f });
            break;
        default:
            ApplyPose({}, {});
            break;
        }
    }
};
