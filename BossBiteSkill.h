#pragma once

#include "BossAttackStateType.h"
#include "BossPoseSkill.h"


class BossBiteSkill final : public BossPoseSkill
{
public:
    static constexpr BossAttackType Type = BossAttackType::Bite;

    explicit BossBiteSkill(Boss& boss) : BossPoseSkill(boss) {}



private:
    void ApplyPhase(AttackStateType phase, float progress) override
    {
        const Vec3 forward = GetForward();
        switch (phase)
        {
        case AttackStateType::PreAction:
            break;
        case AttackStateType::Aiming:
            break;
        case AttackStateType::Attack:
        {
            SetMotionDuration(AttackStateType::Attack, boss_.GetAnimTotalTime(BossAnimType::PowerUp));
            boss_.SetAnimation(BossAnimType::PowerUp, false);
        }
            break;
        case AttackStateType::Recovery:
        {
            const float remaining = 1.0f - progress;
            ApplyPose(forward * (85.0f * remaining), { 0.65f * remaining, 0.0f, 0.0f });
            break;
        }
        default:
            ApplyPose({}, {});
            break;
        }
    }
};
