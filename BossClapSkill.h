#pragma once

#include "BossAttackType.h"
#include "BossPoseSkill.h"

#include <cmath>

class BossClapSkill final : public BossPoseSkill
{
public:
	static constexpr BossAttackType Type = BossAttackType::Clap;

	explicit BossClapSkill(Boss& boss)
		: BossPoseSkill(boss)
    {
        SetFinishCondition(AttackStateType::PreAction, PhaseFinishCondition::AnimationFinished);
        SetFinishCondition(AttackStateType::Aiming, PhaseFinishCondition::AnimationFinished);
        SetFinishCondition(AttackStateType::Attack, PhaseFinishCondition::AnimationFinished);
        SetFinishCondition(AttackStateType::Return, PhaseFinishCondition::AnimationFinished);

        
    }
private:
    void EnterPhase(AttackStateType phase) override
    {
        switch (phase)
        {
        case AttackStateType::PreAction:
            // AnimTypeをに設定し、アニメーションを再生する
            boss_.SetAnimation(BossAnimType::Clap, false,1.0f);
            boss_.GetAnimation().PlayTo(0.0f);
            break;
        case AttackStateType::Aiming:
            // アニメーションの再生位置を設定する
            boss_.GetAnimation().PlayTo(1.0f);
            break;
        case AttackStateType::Attack:
            // アニメーションの再生位置を設定し、速度を設定する
            boss_.GetAnimation().PlayTo(43.0f);
            break;
        case AttackStateType::Recovery:
            break;
        case AttackStateType::Return:
            // アニメーションの再生を開始する
            boss_.GetAnimation().Start(15.0f);
            break;
        }
    }

    void ApplyPhase(AttackStateType phase, float progress) override
    {
        auto& animation = boss_.GetAnimation();

        switch (phase)
        {
        case AttackStateType::PreAction:
            break;

        case AttackStateType::Aiming:
            break;

        case AttackStateType::Attack:
            break;

        case AttackStateType::Recovery:
            break;

        case AttackStateType::Return:
            break;
        }
    }

    void EndPhase(AttackStateType phase) override
    {
        switch (phase)
        {
        case AttackStateType::PreAction:
            break;
        case AttackStateType::Aiming:
            break;
        case AttackStateType::Attack:
            break;
        case AttackStateType::Recovery:
            break;
        case AttackStateType::Return:
            break;
        }
    }
};