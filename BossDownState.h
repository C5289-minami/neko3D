#pragma once

#include "BossBaseState.h"

class BossDownState final : public BossBaseState
{
public:
    BossDownState(BossStateMachine& stateMachine, Boss& boss);

    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;

private:
    static constexpr float DownDurationSeconds = 5.0f;
    float remainingSeconds_{};
};