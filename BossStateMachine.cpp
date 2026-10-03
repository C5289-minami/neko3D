#include "BossStateMachine.h"
#include "AttackStateMachine.h"

BossStateMachine::BossStateMachine(Boss& boss)
    : boss_(boss),
      idleState_(*this, boss),
      moveState_(*this, boss),
	attackState_(*this, boss)
{
}

void BossStateMachine::Initialize()
{
    currentType_ = BossStateType::None;
    ChangeState(BossStateType::Idle);
}


void BossStateMachine::ChangeState(BossStateType type)
{
    if (currentType_ == type) return;

    State* newState = FindState(type);
    if (!newState) return;

    currentType_ = type;
    StateMachine::SwitchState(newState);
}



State* BossStateMachine::FindState(BossStateType type)
{
    switch (type)
    {
    case BossStateType::Idle:
        return &idleState_;
    case BossStateType::Walk:
        return &moveState_;
	case BossStateType::Attack:
		return &attackState_;
    default:
        return nullptr;
    }
}
