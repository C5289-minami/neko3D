#pragma once

#include "BossAttackStateType.h"
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
        case AttackStateType::Aiming:
            ApplyPose({ 0.0f, -20.0f * progress, 0.0f }, { -0.25f * progress, 0.0f, 0.0f });
            break;
        case AttackStateType::Attack:
            ApplyPose({ 0.0f, -40.0f * progress, 0.0f }, { 0.55f * progress, 0.0f, 0.0f });
            break;
        case AttackStateType::Recovery:
            ApplyPose({ 0.0f, -40.0f * (1.0f - progress), 0.0f }, { 0.55f * (1.0f - progress), 0.0f, 0.0f });
            break;
        default:
            ApplyPose({}, {});
            break;
        }
    }
};
