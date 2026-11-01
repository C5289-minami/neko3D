#pragma once

#include "AttackStateType.h"
#include "Boss.h"
#include "Motion.h"
#include <unordered_map>

enum class PhaseFinishCondition
{
    MotionFinished,          // Motion の終了
    AnimationFinished,      // アニメーションの終了
    AnimationBlendFinished  // ブレンドの終了
};

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

    void BeginPhase(AttackStateType phase) override;
    void UpdatePhase(AttackStateType phase, float deltaTime) override;
    void FinishPhase(AttackStateType phase) override;
    bool IsPhaseFinished(AttackStateType phase) const override;

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
    virtual void EnterPhase(AttackStateType phase) {}
    virtual void ApplyPhase(AttackStateType phase, float progress) = 0;
    virtual void EndPhase(AttackStateType phase) {}

    // フェーズの終了条件を設定する
    void SetFinishCondition(
        AttackStateType phase,
        PhaseFinishCondition condition);

    Boss& boss_;
    // フェーズに対応するMotionを取得
    Motion& GetPhaseMotion(AttackStateType phase);
    const Motion& GetPhaseMotion(AttackStateType phase) const;
private:
    // フェーズごとの終了条件
    std::unordered_map<AttackStateType, PhaseFinishCondition> finishConditions_;

    Motion preActionMotion_{ 0.0f, 1.0f, 0.0f, Motion::Easing::EaseOut };
    Motion aimingMotion_{ 0.0f, 1.0f, 0.0f };
    Motion attackMotion_{ 0.0f, 1.0f, 0.0f, Motion::Easing::EaseInOut };
    Motion recoveryMotion_{ 0.0f, 1.0f, 0.0f, Motion::Easing::EaseOut };
    Motion returnMotion_{ 0.0f, 1.0f, 0.0f };
};