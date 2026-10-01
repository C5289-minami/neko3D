#pragma once
#include "StateMachine.h"
#include "BossStateType.h"

#include "BossIdleState.h"
#include "BossWalkState.h"


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
};
