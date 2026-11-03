#pragma once
#include "BossAttackType.h"
#include <vector>
#include "Vector3.h"    

class Boss;

class BossAI
{
public:
    explicit BossAI(Boss& boss);
    void Reset();
    void Update(float deltaTime);

	void TurnToTarget(Vec3 targetPosition, float deltaTime);
private:
    Boss& boss_;


    struct SkillCooldown
    {
        SkillCooldown(float maxcooldown, BossAttackType type):
            remainingTime_(0.0f),
            cooldownTime_(maxcooldown),
			type_(type)
		{
		}
		float remainingTime_{};// 残りクールダウン時間
        const float cooldownTime_{};// クールダウン時間
        void Update(float deltaTime)
        {
            if (remainingTime_ > 0.0f)
                remainingTime_ -= deltaTime;
		}
		bool IsReady() const { return remainingTime_ <= 0.0f; }

		BossAttackType type_{};
    };

    SkillCooldown clapCooldown_;
    SkillCooldown slamCooldown_;
    SkillCooldown tailWhipCooldown_;


    std::vector<SkillCooldown*> skillCooldowns_{
        &clapCooldown_,
        &slamCooldown_,
        &tailWhipCooldown_
    };


    // 攻撃全体のクールダウン
    float attackCooldown_{};
    float attackCooldownTime_{ 2.0f };
};