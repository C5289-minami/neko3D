#pragma once
#include "StateMachine.h"
#include "BossStateType.h"

#include "BossIdleState.h"
#include "BossWalkState.h"
#include "BossAttackState.h"
#include "BossFlinchState.h"
#include "BossDieState.h"

class Boss;

class BossStateMachine : public StateMachine
{
public:
    explicit BossStateMachine(Boss& boss);

    void Initialize();
    void ChangeState(BossStateType type);
    BossStateType GetCurrentType() const { return currentType_; }

private:
    State* FindState(BossStateType type);

    Boss& boss_;
    BossStateType currentType_{ BossStateType::None };
    BossIdleState idleState_;
    BossWalkState moveState_;
	BossAttackState attackState_;
	BossFlinchState flinchState_;
	BossDieState dieState_;
};
