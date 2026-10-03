#pragma once

#include "BossAttackStateType.h"
#include "BossPoseSkill.h"

#include <cmath>

class BossClapSkill final : public BossPoseSkill
{
public:
    static constexpr BossAttackType Type = BossAttackType::Clap;

    explicit BossClapSkill(Boss& boss) : BossPoseSkill(boss) {}

private:
    void ApplyPhase(AttackStateType phase, float progress) override
    {
        switch (phase)
        {
        case AttackStateType::PreAction:
        case AttackStateType::Aiming:
            ApplyPose({}, { -0.15f * progress, -0.35f * progress, -0.2f * progress });
            break;
        case AttackStateType::Attack:
            ApplyPose({}, { 0.35f * progress, 0.0f,
                0.3f * std::sin(progress * 3.14159265f) });
            break;
        case AttackStateType::Recovery:
            ApplyPose({}, { 0.35f * (1.0f - progress), 0.0f, 0.0f });
            break;
        default:
            ApplyPose({}, {});
            break;
        }
    }
};
