#pragma once

#include "BossAttackStateType.h"
#include "BossPoseSkill.h"

class BossHairBallSkill final : public BossPoseSkill
{
public:
    static constexpr BossAttackType Type = BossAttackType::HairBall;

    explicit BossHairBallSkill(Boss& boss) : BossPoseSkill(boss) {}

private:
    void ApplyPhase(AttackStateType phase, float progress) override
    {
        const Vec3 forward = GetForward();
        switch (phase)
        {
        case AttackStateType::PreAction:
        case AttackStateType::Aiming:
            ApplyPose({ 0.0f, -12.0f * progress, 0.0f }, { -0.3f * progress, 0.0f, 0.0f });
            break;
        case AttackStateType::Attack:
            ApplyPose(forward * (45.0f * progress), { 0.35f * progress, 0.0f, 0.08f * progress });
            break;
        case AttackStateType::Recovery:
        {
            const float remaining = 1.0f - progress;
            ApplyPose(forward * (45.0f * remaining), { 0.35f * remaining, 0.0f, 0.08f * remaining });
            break;
        }
        default:
            ApplyPose({}, {});
            break;
        }
    }
};
