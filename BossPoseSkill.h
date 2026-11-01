#pragma once

#include "AttackStateType.h"
#include "Boss.h"
#include "Motion.h"

class BossPoseSkill : public IAttackSkill
{
public:
    explicit BossPoseSkill(
        Boss& boss,
        float preActionTime = 0.35f,
        float aimingTime = 0.15f,
        float attackTime = 0.3f,
        float recoveryTime = 0.35f,
        float returnTime = 0.1f
    )
        : boss_(boss),
        preActionMotion_(0.0f, 1.0f, preActionTime, Motion::Easing::EaseOut),
        aimingMotion_(0.0f, 1.0f, aimingTime),
        attackMotion_(0.0f, 1.0f, attackTime, Motion::Easing::EaseInOut),
        recoveryMotion_(0.0f, 1.0f, recoveryTime, Motion::Easing::EaseOut),
        returnMotion_(0.0f, 1.0f, returnTime)
    {
    }

    void Reset();

    void OnPreActionEnter() override;
    void OnAimingEnter() override;
    void OnAttackEnter() override;
    void OnRecoveryEnter() override;
    void OnReturnEnter() override;

    void OnPreAction(float deltaTime) override;
    void OnAiming(float deltaTime) override;
    void OnAttack(float deltaTime) override;
    void OnRecovery(float deltaTime) override;
    void OnReturn(float deltaTime) override;

    virtual bool IsPreActionFinished() const override;
    virtual bool IsAimingFinished() const override;
    virtual bool IsAttackFinished() const override;
    virtual bool IsRecoveryFinished() const override;
    virtual bool IsReturnFinished() const override;

    void UpdateAnimation(float deltaTime) override
    {
        boss_.UpdateAnimation(deltaTime);
    }
    void SetPhaseDuration(
        float preAction,
        float aiming,
        float attack,
        float recovery,
        float returnTime
    );
protected:
    virtual void ApplyPhase(AttackStateType phase, float progress) = 0;

    Boss& boss_;
private:
    Motion preActionMotion_{ 0.0f, 1.0f, 0.35f, Motion::Easing::EaseOut };
    Motion aimingMotion_{ 0.0f, 1.0f, 0.15f };
    Motion attackMotion_{ 0.0f, 1.0f, 0.3f, Motion::Easing::EaseInOut };
    Motion recoveryMotion_{ 0.0f, 1.0f, 0.35f, Motion::Easing::EaseOut };
    Motion returnMotion_{ 0.0f, 1.0f, 0.1f };
};