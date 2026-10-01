#pragma once
#include "State.h"

class Boss;
class BossStateMachine;

class BossBaseState : public State
{
public:
    explicit BossBaseState(BossStateMachine& stateMachine, Boss& boss)
        : stateMachine_(stateMachine),
          boss_(boss)
    {
    }

protected:
    BossStateMachine& stateMachine_;
    Boss& boss_;
};
