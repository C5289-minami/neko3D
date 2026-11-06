#pragma once

#include "BossAttackType.h"
#include <vector>
#include "Vector3.h"

class Boss;
class Player;

class BossAI
{
public:
    explicit BossAI(Boss& boss);

    void SetPlayer(Player& player) { player_ = &player; }

    void Reset();
    void Update(float deltaTime);
    bool TurnToTarget(const Vec3& targetPosition, float deltaTime);


private:
    Boss& boss_;
    Player* player_{ nullptr };

    struct SkillCooldown
    {
        SkillCooldown(float cooldown, BossAttackType type)
            : remainingTime_(0.0f),
            cooldown_(cooldown),
            type_(type)
        {
        }

        float remainingTime_{};       // 残りクールダウン
        const float cooldown_{};      // 最大クールダウン
        BossAttackType type_{};

        void Update(float deltaTime)
        {
            remainingTime_ -= deltaTime;
            if (remainingTime_ < 0.0f)
				remainingTime_ = 0.0f;
        }

        bool IsReady() const
        {
            return remainingTime_ <= 0.0f;
        }

        void StartCooldown()
        {
            remainingTime_ = cooldown_;
        }
    };

    SkillCooldown clapCooldown_;
    SkillCooldown slamCooldown_;
    SkillCooldown tailWhipCooldown_;

    std::vector<SkillCooldown*> skillCooldowns_{
        &clapCooldown_,
        &slamCooldown_,
        &tailWhipCooldown_
    };

    // 現在実行中のスキル
    SkillCooldown* activeSkillCooldown_{};

    // 攻撃全体のクールダウン
    float attackCooldown_{};
    float attackCooldownTime_{ 2.0f };

    // スキルが使用できない場合の再確認間隔
    float retryInterval_{ 0.1f };

    bool isSkillActive_{ false };
};