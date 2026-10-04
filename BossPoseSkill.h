#pragma once

#include "AttackStateType.h"
#include "Boss.h"
#include "Motion.h"

#include <cmath>

// ボスの攻撃スキルを管理するための抽象クラス

class BossPoseSkill : public IAttackSkill
{
public:
    explicit BossPoseSkill(Boss& boss)
        : boss_(boss)
    {
    }

    void Reset()
    {
        basePosition_ = boss_.GetPosition();
        baseRotation_ = boss_.GetModelObject().rotation;
        preActionMotion_.Reset();
        aimingMotion_.Reset();
        attackMotion_.Reset();
        recoveryMotion_.Reset();
        returnMotion_.Reset();
    }

    void OnPreAction(float deltaTime) override
    {
        preActionMotion_.Update(deltaTime);
        ApplyPhase(AttackStateType::PreAction, preActionMotion_.GetEasedProgress());
    }

    void OnAiming(float deltaTime) override
    {
        aimingMotion_.Update(deltaTime);
        ApplyPhase(AttackStateType::Aiming, 1.0f);
    }

    void OnAttack(float deltaTime) override
    {
        attackMotion_.Update(deltaTime);
        ApplyPhase(AttackStateType::Attack, attackMotion_.GetEasedProgress());
    }

    void OnRecovery(float deltaTime) override
    {
        recoveryMotion_.Update(deltaTime);
        ApplyPhase(AttackStateType::Recovery, recoveryMotion_.GetEasedProgress());
    }

    void OnReturn(float deltaTime) override
    {
        returnMotion_.Update(deltaTime);
        ApplyPose({}, {});
    }

    bool IsPreActionFinished() const override { return preActionMotion_.IsFinished(); }
    bool IsAimingFinished() const override { return aimingMotion_.IsFinished(); }
    bool IsAttackFinished() const override { return attackMotion_.IsFinished(); }
    bool IsRecoveryFinished() const override { return recoveryMotion_.IsFinished(); }
    bool IsReturnFinished() const override { return returnMotion_.IsFinished(); }

protected:
    virtual void ApplyPhase(AttackStateType phase, float progress) = 0;

    Vec3 GetForward() const
    {
        return { std::sin(baseRotation_.y), 0.0f, std::cos(baseRotation_.y) };
    }

    void ApplyPose(const Vec3& positionOffset, const Vec3& rotationOffset)
    {
        boss_.SetPosition(basePosition_ + positionOffset);
        boss_.GetModelObject().rotation = baseRotation_ + rotationOffset;
    }

    void SetMotionDuration(AttackStateType phase, float duration)
    {
        switch (phase)
        {
        case AttackStateType::PreAction:
            preActionMotion_.SetDuration(duration);
            break;
        case AttackStateType::Aiming:
            aimingMotion_.SetDuration(duration);
            break;
        case AttackStateType::Attack:
            attackMotion_.SetDuration(duration);
            break;
        case AttackStateType::Recovery:
            recoveryMotion_.SetDuration(duration);
            break;
        case AttackStateType::Return:
            returnMotion_.SetDuration(duration);
            break;
        default:
            break;
        }
	}

    Boss& boss_;
private:
    Vec3 basePosition_{};
    Vec3 baseRotation_{};
    Motion preActionMotion_{ 0.0f, 1.0f, 0.35f, Motion::Easing::EaseOut };
    Motion aimingMotion_{ 0.0f, 1.0f, 0.15f };
    Motion attackMotion_{ 0.0f, 1.0f, 0.3f, Motion::Easing::EaseInOut };
    Motion recoveryMotion_{ 0.0f, 1.0f, 0.35f, Motion::Easing::EaseOut };
    Motion returnMotion_{ 0.0f, 1.0f, 0.1f };
};
