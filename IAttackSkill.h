#pragma once

// =================================
// 攻撃スキルのインターフェース
// =================================

class IAttackSkill
{
public:
    virtual ~IAttackSkill() = default;

    // 各フェーズの更新処理
    virtual void OnPreAction(float dt) {}
    virtual void OnAiming(float dt) {}
    virtual void OnAttack(float dt) {}
    virtual void OnRecovery(float dt) {}
    virtual void OnReturn(float dt) {}

    // 各フェーズの終了判定（必要に応じて）
    virtual bool IsPreActionFinished() const { return true; }
    virtual bool IsAimingFinished() const { return true; }
    virtual bool IsAttackFinished() const { return true; }
    virtual bool IsRecoveryFinished() const { return true; }
    virtual bool IsReturnFinished() const { return true; }
};