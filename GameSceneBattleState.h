
#pragma once
#include "GameBaseState.h"

class GameSceneBattleState : public GameBaseState
{
public:
    GameSceneBattleState(GameStateMachine& gameStateMachine, GameScene& gameScene);

    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;

private:
    float battleIntroWaitRemaining_{ 0.0f };
    float battleIntroFinalWaitElapsed_{ 0.0f };
    float bossEntranceSoundRemaining_{ 0.0f };

    bool bossEntranceSoundPlayed_{ false };
    bool battleIntroTurnFinished_{ false };
    bool battleStarted_{ false };

    bool wasTouchingBossBody_ = false;
    bool wasTailWhipActive_ = false;

    static constexpr float BattleIntroWaitDuration = 1.5f;
    static constexpr float BattleIntroFinalWaitDuration = 3.0f;

    // 登場演出開始から「バーン」を鳴らすまでの無音時間。単位は秒。
    static constexpr float BossEntranceSoundDelay = 5.0f;

    static constexpr float BattleIntroStartFov = 55.0f;
    static constexpr float BattleIntroEndFov = 40.0f;
};
