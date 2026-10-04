#pragma once

// =================================
// 攻撃スキルのインターフェース
// =================================

class IAttackSkill
{
public:
    virtual ~IAttackSkill() = default;

    // 各フェーズの更新処理
    virtual void OnPreAction(float) {}
    virtual void OnAiming(float) {}
    virtual void OnAttack(float) {}
    virtual void OnRecovery(float) {}
    virtual void OnReturn(float) {}

    // 各フェーズの終了判定
    virtual bool IsPreActionFinished() const { return true; }
    virtual bool IsAimingFinished() const { return true; }
    virtual bool IsAttackFinished() const { return true; }
    virtual bool IsRecoveryFinished() const { return true; }
    virtual bool IsReturnFinished() const { return true; }


};