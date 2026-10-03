#pragma once
#pragma once

#include "StateMachine.h"
#include "AttackStateType.h"
#include "IAttackSkill.h"
#include "AttackBaseState.h"

class AttackStateMachine : public StateMachine
{
public:
    AttackStateMachine();

    void Initialize();

    // どの攻撃を実行するかセットして開始
    void StartAttack(IAttackSkill* skill);

    void ChangeState(AttackStateType type);
    AttackStateType GetCurrentType() const { return currentType_; }
    bool IsFinished() const { return finished_; }

    // 現在実行中のスキルを取得
    IAttackSkill* GetCurrentSkill() const { return currentSkill_; }

private:
    State* FindState(AttackStateType type);

    AttackStateType currentType_{ AttackStateType::None };
    IAttackSkill* currentSkill_{ nullptr };
    bool finished_{ true };
    AttackBaseState preActionState_;
    AttackBaseState aimingState_;
    AttackBaseState attackState_;
    AttackBaseState recoveryState_;
    AttackBaseState returnState_;
};