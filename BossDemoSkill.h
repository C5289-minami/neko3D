#pragma once

#include "Boss.h"
#include "Motion.h"

#include <cmath>

class BossDemoSkill final : public IAttackSkill
{
public:
    explicit BossDemoSkill(Boss& boss)
        : boss_(boss)
    {
    }

    void Reset()
    {
        basePosition_ = boss_.GetPosition();
        baseRotation_ = boss_.GetModelObject().rotation;
        forward_ = { std::sin(baseRotation_.y), 0.0f, std::cos(baseRotation_.y) };
        preActionMotion_.Reset();
        aimingMotion_.Reset();
        attackMotion_.Reset();
        recoveryMotion_.Reset();
        returnMotion_.Reset();
    }

    void OnPreAction(float deltaTime) override
    {
        preActionMotion_.Update(deltaTime);
        const float progress = preActionMotion_.GetEasedProgress();
        ApplyPose(0.0f, -0.2f * progress, -10.0f * progress);
    }

    void OnAiming(float deltaTime) override
    {
        aimingMotion_.Update(deltaTime);
        ApplyPose(0.0f, -0.2f, -10.0f);
    }

    void OnAttack(float deltaTime) override
    {
        attackMotion_.Update(deltaTime);
        const float progress = attackMotion_.GetEasedProgress();
        ApplyPose(100.0f * progress, -0.2f + 0.9f * progress, 20.0f * progress);
    }

    void OnRecovery(float deltaTime) override
    {
        recoveryMotion_.Update(deltaTime);
        const float remaining = 1.0f - recoveryMotion_.GetEasedProgress();
        ApplyPose(100.0f * remaining, 0.7f * remaining, 20.0f * remaining);
    }

    void OnReturn(float deltaTime) override
    {
        returnMotion_.Update(deltaTime);
        ApplyPose(0.0f, 0.0f, 0.0f);
    }

    bool IsPreActionFinished() const override { return preActionMotion_.IsFinished(); }
    bool IsAimingFinished() const override { return aimingMotion_.IsFinished(); }
    bool IsAttackFinished() const override { return attackMotion_.IsFinished(); }
    bool IsRecoveryFinished() const override { return recoveryMotion_.IsFinished(); }
    bool IsReturnFinished() const override { return returnMotion_.IsFinished(); }

private:
	// 前方へのオフセット、ピッチ角度のオフセット、垂直方向のオフセットを適用してボスの位置と回転を更新する
    void ApplyPose(float forwardOffset, float pitchOffset, float verticalOffset)
    {
        boss_.SetPosition(basePosition_ + forward_ * forwardOffset + Vec3{ 0.0f, verticalOffset, 0.0f });
        boss_.GetModelObject().rotation = {
            baseRotation_.x + pitchOffset,
            baseRotation_.y,
            baseRotation_.z
        };
    }

    Boss& boss_;
    Vec3 basePosition_{};
    Vec3 baseRotation_{};
    Vec3 forward_{ 0.0f, 0.0f, 1.0f };

    Motion preActionMotion_{ 0.0f, 1.0f, 0.45f, Motion::Easing::EaseOut };
    Motion aimingMotion_{ 0.0f, 1.0f, 0.15f };
    Motion attackMotion_{ 0.0f, 1.0f, 0.3f, Motion::Easing::EaseInOut };
    Motion recoveryMotion_{ 0.0f, 1.0f, 0.45f, Motion::Easing::EaseOut };
    Motion returnMotion_{ 0.0f, 1.0f, 0.1f };
};
