#define NOMINMAX

#include "BossAI.h"
#include "Boss.h"
#include <random>
#include <algorithm>
#include "GameContext.h"

// スキルが終わってからクールダウンに変更
// 回転と移動を適応

namespace
{
    std::mt19937 randomEngine{ std::random_device{}() };

    float NormalizeAngle(float angle)
    {
        while (angle > DX_PI_F)
            angle -= DX_TWO_PI_F;

        while (angle < -DX_PI_F)
            angle += DX_TWO_PI_F;

        return angle;
    }
}

BossAI::BossAI(Boss& boss)
    : boss_(boss),
    clapCooldown_(10, BossAttackType::Clap),
    slamCooldown_(10, BossAttackType::Slam),
    tailWhipCooldown_(20, BossAttackType::TailWhip)
{
    attackCooldown_ = 5.0f;
}

void BossAI::Reset()
{
    attackCooldown_ = 0.0f;
    for (auto& cooldown : skillCooldowns_)
        cooldown->remainingTime_ = 0.0f;
}

void BossAI::Update(float deltaTime)
{
    // 個別スキルのクールダウン更新
    for (auto& cooldown : skillCooldowns_)
        cooldown->Update(deltaTime);

    // 攻撃全体のクールダウン更新
    if (attackCooldown_ > 0.0f)
    {
        attackCooldown_ -= deltaTime;
        return;
    }

    // 使用可能なスキルを集める
    std::vector<SkillCooldown*> availableSkills;

    for (auto& cooldown : skillCooldowns_)
    {
        if (cooldown->IsReady())
            availableSkills.push_back(cooldown);
    }

    // 使用可能なスキルがなければ再抽選を待つ
    if (availableSkills.empty())
    {
        attackCooldown_ = attackCooldownTime_;
        return;
    }

    // ランダムにスキルを選択
    std::uniform_int_distribution<int> distribution(
        0, static_cast<int>(availableSkills.size()) - 1
    );

    SkillCooldown* selectedSkill =
        availableSkills[distribution(randomEngine)];

    // スキル発動を試す
    if (boss_.UseSkill(selectedSkill->type_))
    {
        selectedSkill->remainingTime_ = selectedSkill->cooldownTime_;
    }

    // 成功・失敗どちらでも次の攻撃まで待つ
    attackCooldown_ = attackCooldownTime_;

}

void BossAI::TurnToTarget(Vec3 targetPosition, float deltaTime)
{
	Vec3 bossPosition = boss_.GetWorldPosition();

	// ターゲットまでのRadian角度を計算
    float yawError =
        NormalizeAngle(
            std::atan2(targetPosition.x - bossPosition.x,
                targetPosition.z - bossPosition.z)
            - boss_.GetYaw());

    yawError *= -1;

	// yawDiff を -π から π の範囲に正規化
	NormalizeAngle(yawError);

    if (std::abs(yawError) > 0.1f) // ある程度の差がある場合のみ回転
    {
        float rotationSpeed = 0.4f; // 回転速度（ラジアン/秒）
        float rotationAmount = rotationSpeed * deltaTime;
        if (yawError > 0)
            boss_.SetYaw(boss_.GetYaw() + std::min(rotationAmount, yawError));
        else
            boss_.SetYaw(boss_.GetYaw() - std::min(rotationAmount, -yawError));
	}

}

