#define NOMINMAX

#include "BossAI.h"
#include "Boss.h"
#include "Player.h"

#include <random>
#include <algorithm>
#include <cmath>

#include "DxPlus/DxPlus.h"

//プレイヤーが2階にいる時のAI分岐（次の攻撃を「しっぽ」に確定させる

// ===========================================================================
// BossAI 仕様
// ===========================================================================
//
// 【攻撃管理】
// ・使用可能なスキルからランダムに1つ選択して発動する。
// ・スキル実行中は、ほかのスキルを発動しない。
// ・スキルの帰りモーションが終了すると、スキル実行中状態を解除する。
//
// 【個別クールダウン】
// ・クールダウンはスキルの帰りモーション終了後から開始する。
// ・スキル実行中は個別クールダウンを更新しない。
// ・クールダウンが0秒以下になると、そのスキルを再使用できる。
// ・使用可能なスキルのみ抽選対象とする。
//
// 【全体クールダウン】
// ・攻撃終了後、5秒間は次の攻撃を行わない。
// ・全体クールダウン中は個別クールダウンのみ更新する。
// ・使用可能なスキルがない場合は、0.1秒後に再抽選する。
// ・スキル発動に失敗した場合も、0.1秒後に再試行する。
//
// ===========================================================================


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
    clapCooldown_(7.0f, BossAttackType::Clap),
    slamCooldown_(8.0f, BossAttackType::Slam),
    tailWhipCooldown_(14.0f, BossAttackType::TailWhip)
{
    attackCooldownTime_ = 1.0f;
    Reset();
}

void BossAI::Reset()
{
    isSkillActive_ = false;
    activeSkillCooldown_ = nullptr;

    attackCooldown_ = attackCooldownTime_;

    // Reset all skill cooldowns.
    for (auto& cooldown : skillCooldowns_)
        cooldown->remainingTime_ = 0.0f;
}

void BossAI::Update(float deltaTime)
{

    // スキル実行中
    if (isSkillActive_)
    {
        if (boss_.GetActiveSkill() && !boss_.GetActiveSkill()->IsReturnFinished())
            return;

        // 使用したスキルのクールダウンを開始
        if (activeSkillCooldown_)
            activeSkillCooldown_->StartCooldown();

        // スキル終了
        isSkillActive_ = false;
        activeSkillCooldown_ = nullptr;

        // 攻撃後の待機時間を設定
        attackCooldown_ = attackCooldownTime_;
        return;
    }

    // 個別スキルのクールダウン更新
    for (auto& cooldown : skillCooldowns_)
        cooldown->Update(deltaTime);

    // 攻撃全体のクールダウン更新
    if (attackCooldown_ > 0.0f)
    {
        attackCooldown_ = std::max(0.0f, attackCooldown_ - deltaTime);
        return;
    }
    // ============================
	// 攻撃の抽選と発動
	// ============================ 

    // 二階にいる場合は、しっぽ攻撃を優先
    if (player_ != nullptr &&
        (player_->WasLastOnSecondFloor() || player_->IsPillarElevatorActive() ||
            player_->GetCurrentState() == PlayerStateType::Climbing))
    {
        // クールダウンは無効
     /*   if (!tailWhipCooldown_.IsReady())
        {
            attackCooldown_ = retryInterval_;
            return;
        }*/

        if (boss_.UseSkill(BossAttackType::TailWhip))
        {
            activeSkillCooldown_ = &tailWhipCooldown_;
            isSkillActive_ = true;
        }
        else
        {
            attackCooldown_ = retryInterval_;
        }

        return;
    }

    // 使用可能なスキルを集める
    std::vector<SkillCooldown*> availableSkills;

    for (auto& cooldown : skillCooldowns_)
    {
        if (cooldown->IsReady())
            availableSkills.push_back(cooldown);
    }

    // 使用可能なスキルがなければ再確認
    if (availableSkills.empty())
    {
        attackCooldown_ = retryInterval_;
        return;
    }

    // ランダムにスキルを選択
    std::uniform_int_distribution<int> distribution(
        0,
        static_cast<int>(availableSkills.size()) - 1
    );

    SkillCooldown* selectedSkill =
        availableSkills[distribution(randomEngine)];

    // スキル発動
    if (boss_.UseSkill(selectedSkill->type_))
    {
        activeSkillCooldown_ = selectedSkill;
        isSkillActive_ = true;
    }
    else
    {
        // 発動失敗時は少し待って再試行
        attackCooldown_ = retryInterval_;
    }

   
}

bool BossAI::TurnToTarget(const Vec3& targetPosition, float deltaTime)
{
    if (!boss_.IsRotation())
        return true;

    Vec3 bossPosition = boss_.GetWorldPosition();

    float targetYaw = std::atan2(
        targetPosition.x - bossPosition.x,
        targetPosition.z - bossPosition.z
    ) + DX_PI_F;

    targetYaw = NormalizeAngle(targetYaw);

    float yawError = NormalizeAngle(targetYaw - boss_.GetYaw());

    constexpr float finishAngle = 1.0f * DxPlus::Deg2Rad;
    constexpr float maxRotationSpeed = 3.5f;
    constexpr float minRotationSpeed = 0.2f;
    //constexpr float acceleration = 3.0f;

    float error = std::abs(yawError);

    if (error <= finishAngle)
    {
        boss_.SetYaw(targetYaw);
        return true;
    }

    // 目標に近づくほど回転速度を落とす
    float speedRate = std::clamp(error / DX_PI_F, 0.0f, 1.0f);
    float rotationSpeed = minRotationSpeed +
        (maxRotationSpeed - minRotationSpeed) * speedRate;

    float rotationAmount = rotationSpeed * deltaTime;

    boss_.SetYaw(
        boss_.GetYaw() + std::copysign(
            std::min(rotationAmount, error),
            yawError
        )
    );

    return false;
}